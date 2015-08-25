/*
 Copyright 2013 Software Reliability Lab, ETH Zurich

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "ActionLog.h"
#include "RaceInfo.h"
#include <iostream>
#include <assert.h>
using namespace std;

const char* ActionLog::CommandType_AsString(CommandType ctype) {
	switch (ctype) {
	case ENTER_SCOPE:
		return "ENTER_SCOPE";
	case EXIT_SCOPE:
		return "EXIT_SCOPE";
	case READ_MEMORY:
		return "READ_MEMORY";
	case WRITE_MEMORY:
		return "WRITE_MEMORY";
	case TRIGGER_ARC:
		return "TRIGGER_ARC";
	case MEMORY_VALUE:
		return "MEMORY_VALUE";
	}
	return "CommandType:OTHER";
}

const char* ActionLog::EventActionType_AsString(EventActionType otype) {
	switch (otype) {
	case UNKNOWN:
		return "_";
	case TIMER:
		return "TIMER";
	case USER_INTERFACE:
		return "USER_INTERFACE";
	case NETWORK:
		return "NETWORK";
	case CONTINUATION:
		return "CONTINUATION";
	}
	return "OperationType:OTHER";
}

ActionLog::ActionLog() :
		m_maxEventActionId(-1), m_currentEventActionId(-1) {
}

ActionLog::~ActionLog() {
	for (std::map<int, EventAction*>::iterator it = m_eventActions.begin();
			it != m_eventActions.end(); ++it) {
		delete it->second;
	}
}

void ActionLog::addArc(int earlierOperation, int laterOperation,
		int arcDuration) {
	Arc a;
	a.from = earlierOperation;
	a.to = laterOperation;
	a.m_duration = arcDuration;
	m_arcs.push_back(a);
}

void ActionLog::startEventAction(int operation) {
	m_currentEventActionId = operation;
	if (m_eventActions[m_currentEventActionId] == NULL) {
		m_eventActions[m_currentEventActionId] = new EventAction();
	}
	if (operation > m_maxEventActionId) {
		m_maxEventActionId = operation;
	}
	m_cmdsInCurrentEvent.clear();
}

bool ActionLog::endEventAction() {
	bool wasInOp = m_currentEventActionId != -1;
	m_currentEventActionId = -1;
	m_cmdsInCurrentEvent.clear();
	return wasInOp;
}

bool ActionLog::setEventActionType(EventActionType op_type) {
	if (m_currentEventActionId == -1)
		return false;
	m_eventActions[m_currentEventActionId]->m_type = op_type;
	return true;
}

bool ActionLog::willLogCommand(CommandType command) {
	if (m_currentEventActionId == -1)
		return false;
	std::vector<Command>& current_cmds =
			m_eventActions[m_currentEventActionId]->m_commands;
	if (command == MEMORY_VALUE) {
		if (current_cmds.size() == 0)
			return false;
		Command& lastc = current_cmds[current_cmds.size() - 1];
		if (lastc.m_cmdType != READ_MEMORY && lastc.m_cmdType != WRITE_MEMORY) {
			return false;
		}
	}
	return true;
}

bool ActionLog::logCommand(CommandType command, int memoryLocation) {
	if (m_currentEventActionId == -1)
		return false;
	if (!willLogCommand(command))
		return true;
	Command c;
	c.m_cmdType = command;
	c.m_location = memoryLocation;
	if (command == READ_MEMORY || command == WRITE_MEMORY) {
		if (!m_cmdsInCurrentEvent.insert(c).second) {
			return true; // Already exists, no need to add again to the same op.
		}
	}
	std::vector<Command>& current_cmds =
			m_eventActions[m_currentEventActionId]->m_commands;
	if (command == EXIT_SCOPE && current_cmds.size() > 0
			&& current_cmds[current_cmds.size() - 1].m_cmdType == ENTER_SCOPE) {
		current_cmds.pop_back(); // Remove the last enter scope. There was nothing in it and we exit it.
		return true;
	}
	current_cmds.push_back(c);
	return true;
}

struct ActionLogHeader {
	int num_ops;
	int num_arcs;
};

struct OperationHeader {
	int id;
	ActionLog::EventActionType type;
	int num_commands;
};

void ActionLog::saveToFile(FILE* f) {
	ActionLogHeader hdr;
	hdr.num_arcs = m_arcs.size();
	hdr.num_ops = m_eventActions.size();
	fwrite(&hdr, sizeof(hdr), 1, f);
	fwrite(m_arcs.data(), sizeof(Arc), m_arcs.size(), f);
	for (EventActionSet::const_iterator it = m_eventActions.begin();
			it != m_eventActions.end(); ++it) {
		OperationHeader ophdr;
		const EventAction& op = *(it->second);
		ophdr.id = it->first;
		ophdr.type = op.m_type;
		ophdr.num_commands = op.m_commands.size();
		fwrite(&ophdr, sizeof(ophdr), 1, f);
		fwrite(op.m_commands.data(), sizeof(Command), op.m_commands.size(), f);
	}
	fflush(f);
	printf("Action log saved.\n");
}

bool ActionLog::loadFromFile(FILE* f) {
	cout << "\n";
	ActionLogHeader hdr;
	if (fread(&hdr, sizeof(hdr), 1, f) != 1)
		return false;
	m_arcs.resize(hdr.num_arcs);
	if (fread(m_arcs.data(), sizeof(Arc), m_arcs.size(), f) != m_arcs.size())
		return false;
	for (int i = 0; i < hdr.num_ops; ++i) {  //number of operations
		OperationHeader ophdr;
		if (fread(&ophdr, sizeof(ophdr), 1, f) != 1)
			return false;
		EventAction* op = new EventAction();  //for each operation(event action)
		op->m_commands.resize(ophdr.num_commands); //number of commands in the operatsion
		op->m_type = ophdr.type;
		if (fread(op->m_commands.data(), sizeof(Command), op->m_commands.size(),
				f) != op->m_commands.size()) {
			return false;
		}
//		printf("Event: %d,  # of commands: %ld\n", ophdr.id,
//				op->m_commands.size());
		m_eventActions[ophdr.id] = op;
		if (ophdr.id > m_maxEventActionId) {
			m_maxEventActionId = ophdr.id;
		}
	}

	EventActionSet::iterator it;
	for (it = m_eventActions.begin(); it != m_eventActions.end(); ++it) {
//		cout << "Event Id: " << it->first << "  Commands: "
//				<< it->second->m_commands.size() << "\n";
		RaceHandler::Instance()->EventActionSet[it->first] =
				it->second->m_commands.size();
	}

	for (size_t i = 0; i < m_arcs.size(); ++i) {
//		printf("Arc:  %d ->  %d\n", m_arcs[i].from, m_arcs[i].to);
//		RaceInfo::Instance()->AcrSet[m_arcs[i].from].push_back(m_arcs[i].to);
//		RaceInfo::Instance()->preds[m_arcs[i].to].push_back(m_arcs[i].from);
		if (m_arcs[i].to > m_maxEventActionId)
			m_maxEventActionId = m_arcs[i].to;
		if (m_arcs[i].from > m_maxEventActionId)
			m_maxEventActionId = m_arcs[i].from;
	}
	return true;
}

