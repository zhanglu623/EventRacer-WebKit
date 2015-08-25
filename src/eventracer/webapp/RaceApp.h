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

#ifndef RACEAPP_H_
#define RACEAPP_H_

#include <string>
#include <vector>
#include "base.h"
#include "CallTraceBuilder.h"
#include "EventGraph.h"
#include "EventGraphInfo.h"
#include "StringSet.h"
#include "ActionLog.h"
#include "VarsInfo.h"
#include "RaceTags.h"
#include "RaceInfo.h"

class ActionLogPrinter;
class EventGraphDisplay;
class URLParams;
class CodeOutput;

class RaceApp {
public:
	RaceApp(int64 app_id, const std::string& actionLogFile);
	~RaceApp();

	void handleInfo(const std::string& params, std::string* response);

	void handleVarList(const std::string& params, std::string* response);

	void handleRaceChildren(const std::string& params, std::string* response);

	void handleVarDetails(const std::string& params, std::string* response);

	void handleRaceDetails(const std::string& params, std::string* response);

	void handleBrowseGraph(const std::string& params, std::string* response);

	void handleShowCode(const std::string& params, std::string* response);

	void handleShowJS(const std::string& params, std::string* response);

	void handleNodeRelation(const std::string& params, std::string* response);

	void handleUndefRaces(const std::string& params, std::string* response);

	pair<string, pair<string, string> > getRaceInfoFromString(
			vector<string> *ops, int eid, string fullEvent);


	bool checkSpecialEventHandler(string e1, string e2);

	string isFiringEvent(vector<string> *ops);

	string isParsingDOM(vector<string> *ops);

	string isTimerEvent(vector<string> *ops);

	string isParsingScript(vector<string> *ops);

	string isNetWorkEvent(vector<string> *ops);

	string isScriptRunner(vector<string> *ops);

public:
	int replayableRacesCount;
	int uncovered;
	int duplicated;
	int non_replayable;
	int ajax_race;
	int sync_race;
	int bogus_race;
	int closure_race;
	int bubble_race;
	int onmouse_race;
	int no_info;

	int timer_race;
	int getVarFilterLevel(int var_id, const VarsInfo::VarData& data) const;

	void showEventsSummariesIntoTable(const std::vector<int>& events,
			std::string* response);

	std::string raceSetStr(const std::vector<int>& races) const;

	std::string getRaceVars(const std::vector<int>& races) const;

	void showRaceLink(int race_id, std::string* response) const;

	void showRaceInfo(int race_id, std::string* response) const;

	// Shows a stack trace of an action with the action_str put as innermost string.
	void printVarAccessCallTrace(const VarsInfo::VarAccess& var_access,
			const std::string& action_str, std::string* response);

	void Lu_printVarAccessCallTrace(const VarsInfo::VarAccess& var_access,
			const std::string& action_str, std::string* response);

	void printCommandCallTrace(int event_id, int cmd_id, CodeOutput* code);

	void Lu_printAllCommandCallTrace(int event_id, int cmd_id,
			CodeOutput* code);

	void printCommandCallTraceFromPreviousEvents(int event_id, int cmd_id,
			CodeOutput* code, int trigger_depth);

	void Lu_printCommandCallTraceFromPreviousEvents(int event_id, int cmd_id,
			CodeOutput* code, int trigger_depth);

	std::string eventActionAsStr(int event_action_id) const;

	std::string getVarTagsString(int var_id) const;

	void displayRacesIfEnabled(const URLParams& url,
			EventGraphDisplay* graph) const;

	void displayNodeRelation(int node1, int node2, int cmd_in_node2,
			std::string* response) const;

	void displaySearchBox(const std::string& var_name, int filter_level,
			std::string* response) const;

	bool getAccessValue(int event_action_id, int command_id,
			std::string* value) const;

	int64 m_appId;

	ActionLog m_actions;
	StringSet m_vars;
	StringSet m_scopes;
	StringSet m_js;
	StringSet m_memValues;

	VarsInfo m_vinfo;
	RaceTags m_raceTags;
	EventGraphInfo m_graphInfo;
	SimpleDirectedGraph m_inputEventGraph;
	CallTraceBuilder m_callTraceBuilder;
	SimpleDirectedGraph m_graphWithTimers;
	ActionLogPrinter* m_actionPrinter;

	std::string m_fileName;
};

#endif /* RACEAPP_H_ */
