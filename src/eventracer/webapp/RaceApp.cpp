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

#include <stdarg.h> // For va_list and related operations
#include <stdio.h>
#include <algorithm>
#include "base.h"
#include "stringprintf.h"
#include "strutil.h"

#include "ActionLogPrint.h"
#include "Escaping.h"
#include "EventGraphViz.h"
#include "GraphFix.h"
#include "GraphViz.h"
#include "HTMLTable.h"
#include "RaceApp.h"
#include "UrlEncoding.h"
#include "TimerGraph.h"
#include "ThreadMapping.h"
#include "JsViewer.h"
#include <sstream>
//#include "../input/RaceInfo.h"

#include <string.h>

#include <algorithm>
#include <utility>
#include <queue>

using std::string;

namespace {
void addCSS(string* response) {
	response->append(
			"<style type=\"text/css\">\n"
					".ru0 {\n"
					"  background-color:#ffe\n"
					"}\n"
					".ru1 {\n"
					"  background-color:#eed\n"
					"}\n"
					".rk0 {\n"
					"  background-color:#9f9\n"
					"}\n"
					".rk1 {\n"
					"  background-color:#8e8\n"
					"}\n"
					".rs0 {\n"
					"  background-color:#fba\n"
					"}\n"
					".rs1 {\n"
					"  background-color:#e89\n"
					"}\n"

					".ru0:hover {\n"
					"  background-color:#ccb\n"
					"}\n"
					".ru1:hover {\n"
					"  background-color:#ccb\n"
					"}\n"
					".rk0:hover {\n"
					"  background-color:#7d7\n"
					"}\n"
					".rk1:hover {\n"
					"  background-color:#6c6\n"
					"}\n"
					".rs0:hover {\n"
					"  background-color:#d98\n"
					"}\n"
					".rs1:hover {\n"
					"  background-color:#c79\n"
					"}\n"

					".blue {\n"
					"  color:#22e\n"
					"}\n"

					".clickable { cursor:pointer }\n"

					".hiddenrow {\n"
					"   overflow:hidden;display:none \n"
					"}\n"
					".visiblerow {\n"
					"   overflow:hidden; \n"
					"}\n"

					".visibled {\n"
					"    overflow:hidden; margin: 0em 0em 0em 0em; transition: margin 0.05s ease-in-out\n"
					"}\n"
					".hiddend {\n"
					"    overflow:hidden; margin: -2.8em 0em 0em 0em; transition: margin 0.05s ease-in-out\n"
					"}\n"

					".padparagraph {\n"
					"  padding: 0px 0px 0px 30px\n"
					"}\n"

					"</style>\n");
}

void addHeader(string* response, const string& title) {
	StringAppendF(response,
			"<html>"
					"<head><title>EventRacer: %s</title>"
					"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
					"</head>", HTMLEscape(title).c_str());
	response->append("<body>");
	addCSS(response);
	StringAppendF(response, "<h1>%s</h1>", HTMLEscape(title).c_str());
}

void addFooter(string* response) {
	response->append(
			"<div class=\"footer\"><br><br>"
					"EventRacer is a tool of ETH Zurich.<br>For more information, visit us at <a href=\"http://eventracer.org/\">http://eventracer.org/</a>"
					"</div></body></html>\n");
}

const char* VarTypeByName(const char* name) {
	if (strncmp(name, "Tree[", 5) == 0) {
		return "DOM Node";
	}
	if (strncmp(name, "DOMNode[", 8) == 0) {
		return "DOM Attribute";
	}
	if (strncmp(name, "Array[", 6) == 0) {
		return "JS Array";
	}
	return "JS Variable";
}

// End utility
}// namespace

RaceApp::RaceApp(int64 app_id, const std::string& actionLogFile) :
		m_appId(app_id), m_raceTags(m_vinfo, m_actions, m_vars, m_scopes,
				m_memValues, m_callTraceBuilder), m_fileName(actionLogFile) {
	fprintf(stderr, "Loading %s... ", actionLogFile.c_str());

//	RaceHandler::Instance()->readZlLog("../webkit/zlLog.txt");
	RaceHandler::Instance()->readToken("../webkit/DebugInfo/token"); //Tokens[token.eid] = token;
//	RaceHandler::Instance()->readTimer("../webkit/DebugInfo/timer"); //get all the timer events,timerEvents[currEid] = timerType;
//	RaceHandler::Instance()->readNetwork("../webkit/DebugInfo/network");
//	RaceHandler::Instance()->readEvent("../webkit/DebugInfo/event");
	RaceHandler::Instance()->readIds("../webkit/DebugInfo/id");

	FILE* f = fopen(actionLogFile.c_str(), "rb");
	if (!f) {
		fprintf(stderr, "Cannot open file %s\n", actionLogFile.c_str());
		exit(1);
		return;
	}
	m_vars.loadFromFile(f);

	m_scopes.loadFromFile(f);
	m_actions.loadFromFile(f);
	if (!feof(f)) {
		m_js.loadFromFile(f);
	}
	if (!feof(f)) {
		m_memValues.loadFromFile(f);
	}
	fclose(f);
	fprintf(stderr, "DONE\n");

	m_inputEventGraph.addNodesUpTo(m_actions.maxEventActionId());
	int num_arcs = 0;
	for (size_t i = 0; i < m_actions.arcs().size(); ++i) { //add arc information to nodes
		const ActionLog::Arc& arc = m_actions.arcs()[i];
		if (arc.from > arc.to) {
			fprintf(stderr, "Unexpected backwards arc %d -> %d\n", arc.from,
					arc.to);
		}
		m_inputEventGraph.addArc(arc.from, arc.to);

//		std::string timerType = "NotTimer";
//		if (RaceHandler::Instance()->isTimer(arc.to)) {
//			timerType = RaceHandler::Instance()->timerEvents[arc.to].timerType;
//		}
//		if (RaceHandler::Instance()->networkEvents.find(arc.to)
//				!= RaceHandler::Instance()->networkEvents.end()) {
//
//			timerType = RaceHandler::Instance()->networkEvents[arc.to];
//
//		}

//		if (RaceHandler::Instance()->succSet[arc.from].find(arc.to)
//				== RaceHandler::Instance()->succSet[arc.from].end()) {
//
//			RaceHandler::Instance()->succSetTypeIndex[arc.from][timerType][arc.to] =
//					RaceHandler::Instance()->succSetTypeIndex[arc.from][timerType].size();
//
////			cout << "Pred: " << arc.from << "  ,  child: " << arc.to << "  |  "
////					<< "  ,  type: " << timerType << "  , Index: "
////					<< RaceHandler::Instance()->succSetTypeIndex[arc.from][timerType][arc.to]
////					<< "\n";
//
//			RaceHandler::Instance()->succSet[arc.from][arc.to] =
//					make_pair(timerType,
//							RaceHandler::Instance()->succSetTypeIndex[arc.from][timerType][arc.to]);
//
////			RaceHandler::Instance()->succSet[arc.from].insert(arc.to);
//		}

//		if (RaceHandler::Instance()->predSet[arc.to].find(arc.from)
//				== RaceHandler::Instance()->predSet[arc.to].end()) {
//			RaceHandler::Instance()->predSet[arc.to][arc.from] =
//					RaceHandler::Instance()->predSet[arc.to].size();
////			RaceHandler::Instance()->predSet[arc.to].insert(arc.from);
//		}

		++num_arcs;
//		cout<<"From: "<<arc.from<<"  To: "<<arc.to<<"  Duration: "<<arc.m_duration<<"\n";
	}

	printf("Created graph with %d nodes, %d arcs.\n",
			m_inputEventGraph.numNodes(), num_arcs);

	m_callTraceBuilder.Init(m_actions, m_inputEventGraph);

	m_graphInfo.init(m_actions);
	EventGraphFixer fixer(&m_actions, &m_vars, &m_scopes, &m_inputEventGraph,
			&m_graphInfo);
	fixer.dropNoFollowerEmptyEvents();
	fixer.makeIndependentEventExploration();
	fixer.addScriptsAndResourcesHappensBefore();
	fixer.addEventAfterTargetHappensBefore();
	m_vinfo.init(m_actions);
	printf("All variables loaded.\n");

	printf("Building timers graph...\n");
	int64 start_time = GetCurrentTimeMicros();

	//lu: using timer arcs
	m_graphWithTimers = m_inputEventGraph;

	TimerGraph timerg(m_actions.arcs(), m_graphWithTimers);
	timerg.build(&m_graphWithTimers);
	printf("Timers graph done (%lld ms).\n",
			(GetCurrentTimeMicros() - start_time) / 1000);

	printf("Checking for races...\n");
	start_time = GetCurrentTimeMicros();

	m_vinfo.findRaces(m_actions, m_graphWithTimers);

	printf("Done checking for races (%lld ms)...\n",
			(GetCurrentTimeMicros() - start_time) / 1000);

//	RaceHandler::Instance()->findRaceThreadInfo(m_actions, m_inputEventGraph,
//			m_graphWithTimers);

	m_actionPrinter = new ActionLogPrinter(&m_actions, &m_vars, &m_scopes,
			&m_memValues);
///////////////////////////////////////////////pre scan////////////////////////////////////////////////////////////////
	map<pair<int, int>, int> existing_races;
//	int uncovered = 0;
//	int duplicated = 0;
//	int non_replayable = 0;
//	int ajax_race = 0;
//	int sync_race = 0;
//	int timer_race = 0;

	int count_pre_run = 0;
	if (true) {

		int race_size = m_vinfo.m_races.size();
		int ii = 0;
		for (std::vector<VarsInfo::Race_Info>::iterator it =
				m_vinfo.m_races.begin(); it != m_vinfo.m_races.end(); ++it) {

			ii++;
			string var_name = m_vars.getString((*it).m_varId);
			pair<int, int> curr_race = make_pair((*it).m_event1,
					(*it).m_event2);
			if (existing_races.find(curr_race) != existing_races.end()) {
				if ((*it).m_coveredBy == -1) {
					cout << "--Var is:  " << (*it).m_varId << " : " << var_name
							<< endl;
					cout << (*it).m_event1 << "  VS  " << (*it).m_event2
							<< endl;
					duplicated++;
					cout << (*it).m_event1 << " -> " << (*it).m_event2
							<< " duplicated!" << endl;
				}
				continue;
			}

			if ((*it).m_coveredBy != -1) {
//				cout << "Race is covered by: " << (*it).m_coveredBy << " , skip"
//						<< endl;
				continue;//if this race is covered by some other races, then skip.
			} else {
				uncovered++;
//				cout << "Race is Not covered" << endl;
			}

//			cout << "-------------------New Race Info: " << ii << "/"
//					<< race_size << "---------------------------" << endl;
//			cout << "\n\n--Race Info for latest replay" << endl;
//			cout << "--Var is:  " << (*it).m_varId << " : " << var_name << endl;
//			cout << (*it).m_event1 << "  VS  " << (*it).m_event2 << endl;

			bool skipRace = true;

			if (var_name.find("].DOMContentLoaded") != std::string::npos) {

				cout << "-------------------New Race Info: " << ii << "/"
						<< race_size << "---------------------------" << endl;
				cout << "\n\n--Race Info for latest replay" << endl;
				cout << "--Var is:  " << (*it).m_varId << " : " << var_name
						<< endl;
				cout << (*it).m_event1 << "  VS  " << (*it).m_event2 << endl;

				string get_DOM_ptr = var_name.substr(10,
						var_name.length() - 28);

				cout << get_DOM_ptr << endl;

				RaceHandler::Instance()->ptr_id[get_DOM_ptr] = "Lu_DOM";

				RaceHandler::Instance()->id_ptr["Lu_DOM"] = get_DOM_ptr;

				var_name = "Lu_DOM__onDOMContentLoaded";

				cout << "Race Var matches : document.onDOMContentLoaded"
						<< endl;

				skipRace = false;

			} else {

				map<string, string>::iterator it_ptr_id;
				for (it_ptr_id = RaceHandler::Instance()->ptr_id.begin();
						it_ptr_id != RaceHandler::Instance()->ptr_id.end();
						++it_ptr_id) {
					string _key = (*it_ptr_id).first;
					string _value = (*it_ptr_id).second;

					if (var_name.find(_key) != std::string::npos) {

						cout << "-------------------New Race Info: " << ii
								<< "/" << race_size
								<< "---------------------------" << endl;
						cout << "\n\n--Race Info for latest replay" << endl;
						cout << "--Var is:  " << (*it).m_varId << " : "
								<< var_name << endl;
						cout << (*it).m_event1 << "  VS  " << (*it).m_event2
								<< endl;

						cout << "Race Var matches ID: \n" << _key << "  ->  "
								<< _value << endl;

						vector<string> split_var =
								RaceHandler::Instance()->split_string(var_name,
										".");
						var_name = _value + "__on" + split_var[1];
						skipRace = false;
						break;
					}

				}
			}

			if (skipRace == true) {
				continue;
			} else {
				existing_races[curr_race] = 1;

			}

			cout << "Pre scan check : " << var_name << endl;

			VarsInfo::AllVarData::const_iterator var_it =
					m_vinfo.variables().find((*it).m_varId);
			const VarsInfo::VarData& var_data = var_it->second;
			const VarsInfo::VarAccess* loc1 = var_data.findAccessLocation(
					(*it).m_access1 == VarsInfo::MEMORY_READ, (*it).m_event1);
			const VarsInfo::VarAccess* loc2 = var_data.findAccessLocation(
					(*it).m_access2 == VarsInfo::MEMORY_READ, (*it).m_event2);

			if (loc1 != NULL && loc2 != NULL) {
//			cout << "--Event1:  " << (*it).m_event1 << endl;
				std::string s1, s2;
				std::string ss1, ss2;
				printVarAccessCallTrace(*loc1,
						StringPrintf("%s <b>%s</b>",
								VarsInfo::Race_Info::AccessStr((*it).m_access1),
								HTMLEscape(var_name).c_str()), &s1);
				m_actionPrinter->printEventActionDetails((*it).m_event1, &ss1);
//				cout << "--Event1:  " << (*it).m_event1 << "\n" << s1 << endl;
//				cout << "--Full Event1:  " << (*it).m_event1 << "\n" << ss1
//						<< endl;
				cout << "------------------------------------------------"
						<< endl;

//			cout << "--OP1: " << s << endl;

				vector<string> op1 = RaceHandler::Instance()->split_string(s1,
						"***");

//			for (vector<string>::iterator itOp1 = op1.begin();
//					itOp1 != op1.end(); ++itOp1) {
//				cout << "---" << (*itOp1) << endl;
//			}
				pair<string, pair<string, string> > RaceInformation1;
				if (RaceHandler::Instance()->op_to_raceInfo.find((*it).m_event1)
						!= RaceHandler::Instance()->op_to_raceInfo.end()) {
					RaceInformation1 =
							RaceHandler::Instance()->op_to_raceInfo[(*it).m_event1];
				} else {
					RaceInformation1 = getRaceInfoFromString(&op1,
							(*it).m_event1, ss1);
					///lu: don't record event since for same eit, it might be differnet, for bubble events
//					RaceHandler::Instance()->op_to_raceInfo[(*it).m_event1] =
//							RaceInformation1;
				}

//			cout << "\n--Event2:  " << (*it).m_event2 << endl;

//				cout << "\n-----------end of event 1---------------\n" << endl;

				printVarAccessCallTrace(*loc2,
						StringPrintf("%s <b>%s</b>",
								VarsInfo::Race_Info::AccessStr((*it).m_access2),
								HTMLEscape(var_name).c_str()), &s2);

				m_actionPrinter->printEventActionDetails((*it).m_event2, &ss2);
//				cout << "--Event2:  " << (*it).m_event2 << "\n" << s2 << endl;

//				cout << "--Full Event2:  " << (*it).m_event2 << "\n" << ss2
//						<< endl;

				cout << "------------------------------------------------"
						<< endl;
//			cout << "--OP2: " << s << endl;
				vector<string> op2 = RaceHandler::Instance()->split_string(s2,
						"***");
//			for (vector<string>::iterator itOp2 = op2.begin();
//					itOp2 != op2.end(); ++itOp2) {
//				cout << "---" << (*itOp2) << endl;
//			}
//			pair<string, pair<string, string> > RaceInformation2 = getRaceInfo(
//					&op2, (*it).m_event2);

				pair<string, pair<string, string> > RaceInformation2;
				if (RaceHandler::Instance()->op_to_raceInfo.find((*it).m_event2)
						!= RaceHandler::Instance()->op_to_raceInfo.end()) {
					RaceInformation2 =
							RaceHandler::Instance()->op_to_raceInfo[(*it).m_event2];
				} else {
					RaceInformation2 = getRaceInfoFromString(&op2,
							(*it).m_event2, ss2);
					///lu: don't record event since for same eit, it might be differnet, for bubble events
//					RaceHandler::Instance()->op_to_raceInfo[(*it).m_event2] =
//							RaceInformation2;
				}

				if (RaceInformation1.first.compare("NULL") != 0
						&& RaceInformation2.first.compare("NULL") != 0) {

					cout << "Event 1 Id: " << RaceInformation1.second.first
							<< endl;
					cout << "Event 1 Tp: " << RaceInformation1.second.second
							<< endl;

					cout << "Event 2 Id: " << RaceInformation2.second.first
							<< endl;
					cout << "Event 2 Tp: " << RaceInformation2.second.second
							<< endl;

					bool toContinue = false;

					if (RaceInformation1.second.first.empty()) {
						if (RaceInformation1.second.second.compare(
								"onDOMContentLoaded") == 0) {
							RaceInformation1.second.first = "Lu_DOM";
						} else {
							cout<<"no info race"<<endl;
							no_info++;
//							toContinue = true;
							continue;
						}
					}
					if (RaceInformation2.second.first.empty()) {
						if (RaceInformation2.second.second.compare(
								"onDOMContentLoaded") == 0) {
							RaceInformation2.second.first = "Lu_DOM";
						} else {
							no_info++;
							cout<<"no info race"<<endl;
//							toContinue = true;
							continue;
						}
					}
					if (RaceInformation1.second.second.empty()) {
//						toContinue = true;
						continue;
					}
					if (RaceInformation2.second.second.empty())
//						toContinue = true;
						continue;

					if (RaceInformation1.second.second.compare(
							"onreadystatechange") == 0) {
//						toContinue = true;
						ajax_race++;
						continue;
					}

					if (RaceInformation2.second.second.compare(
							"onreadystatechange") == 0) {
//						toContinue = true;
						ajax_race++;
						continue;
					}

					if (RaceInformation1.second.second.find("__BUBBLE")
							!= string::npos
							|| RaceInformation1.second.second.find("__CAPTURE")
									!= string::npos) {
//						toContinue = true;
						bubble_race++;
						continue;
					} else if (RaceInformation2.second.second.find("__BUBBLE")
							!= string::npos
							|| RaceInformation2.second.second.find("__CAPTURE")
									!= string::npos) {
//						toContinue = true;
						bubble_race++;
						continue;
					}

					//if both of the two events are replayable

					if (var_name.find("Lu_DOM__onclick") != std::string::npos) {
						bubble_race++;
						continue;
					}

					string Event1String = "" + RaceInformation1.second.first
							+ "__" + RaceInformation1.second.second;

					string Event2String = "" + RaceInformation2.second.first
							+ "__" + RaceInformation2.second.second;

					cout << "e1: " << Event1String << endl;
					cout << "e2: " << Event2String << endl;

					if (checkSpecialEventHandler(Event1String, Event2String)) {
						bogus_race++;
//						toContinue = true;
						cout << "Bogus race!" << endl;
						continue;
					} else if (checkSpecialEventHandler(Event2String,
							Event1String)) {
						bogus_race++;
//						toContinue = true;
						cout << "Bogus race!" << endl;
						continue;
					}

					if (toContinue) {
//						cout << "Not replayable!" << endl;
						continue;
					}

					if ((*it).m_event2 <= 17 || (*it).m_event1 <= 17) {
						bogus_race++;
						continue;
					}

					if (RaceInformation1.second.first.empty()
							|| RaceInformation2.second.first.empty()
							|| RaceInformation1.second.second.empty()
							|| RaceInformation2.second.second.empty()) {
						no_info++;
						cout<<"no info race"<<endl;
						continue;

					}
					++replayableRacesCount;
					string raceTemplete =
							RaceHandler::Instance()->readRaceTemplete(
									"raceTemplete.txt");
					string raceTemplete_B = raceTemplete;
					//e.g., race1a

					stringstream ss;
					ss << replayableRacesCount;

					RaceHandler::Instance()->str_replace(raceTemplete,
							"@raceVar@", var_name);
					{
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@raceId@", ss.str());
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@raceAorB@", "a");
						//event 1 id
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@event1Id@",
								RaceInformation1.second.first.c_str());
						//event 1 type
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@event1Type@",
								RaceInformation1.second.second.c_str());
						//event 2 id
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@event2Id@",
								RaceInformation2.second.first.c_str());
						//event 2 type
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@event2Type@",
								RaceInformation2.second.second.c_str());
					}
					RaceHandler::Instance()->str_replace(raceTemplete_B,
							"@raceVar@", var_name);
					{
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@raceId@", ss.str());
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@raceAorB@", "b");
						//event 2 id
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@event2Id@",
								RaceInformation1.second.first.c_str());
						//event 2 type
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@event2Type@",
								RaceInformation1.second.second.c_str());
						//event 1 id
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@event1Id@",
								RaceInformation2.second.first.c_str());
						//event 1 type
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@event1Type@",
								RaceInformation2.second.second.c_str());
					}
					cout << raceTemplete << endl;

					cout << raceTemplete_B << endl;

					char fileA[50];
					sprintf(fileA, "race%d%c.txt", replayableRacesCount, 'a');

					char fileB[50];
					sprintf(fileB, "race%d%c.txt", replayableRacesCount, 'b');

					RaceHandler::Instance()->logToRace(fileA, raceTemplete);
					RaceHandler::Instance()->logToRace(fileB, raceTemplete_B);

				}

				else {
					vector<string>::iterator it_op;

					bool to_break = false;

					for (it_op = op1.begin(); it_op != op1.end(); ++it_op) {
						if ((*it_op).find("event:") != string::npos) {
							no_info++;
							to_break = true;
							cout<<"no info race"<<endl;
							break;
						}
					}

					if (to_break == true)
						continue;

					for (it_op = op2.begin(); it_op != op2.end(); ++it_op) {
						if ((*it_op).find("event:") != string::npos) {
							no_info++;
							to_break = true;
							cout<<"no info race"<<endl;
							break;
						}
					}
					if (to_break == true)
						continue;
					cout << "Not replayable!" << endl;
				}

//				cout << "\n-----------end of event 1---------------\n" << endl;
			}

		}
	}

//	cout << "Uncovered Race Count: " << uncovered << endl;

	cout << "Pre scan ends" << endl;

////////////////////////real race handling//////////////////////////////////////////////////////////////////////////////////////
	if (true) {

//		map<pair<int, int>, int> existing_races;

		int race_size = m_vinfo.m_races.size();
		int ii = 0;
		for (std::vector<VarsInfo::Race_Info>::iterator it =
				m_vinfo.m_races.begin(); it != m_vinfo.m_races.end(); ++it) {

			ii++;

			string var_name = m_vars.getString((*it).m_varId);
			pair<int, int> curr_race = make_pair((*it).m_event1,
					(*it).m_event2);
			if (existing_races.find(curr_race) != existing_races.end()) {
				if ((*it).m_coveredBy == -1) {
					cout << "--Var is:  " << (*it).m_varId << " : " << var_name
							<< endl;
					cout << (*it).m_event1 << "  VS  " << (*it).m_event2
							<< endl;
					duplicated++;
					cout << (*it).m_event1 << " -> " << (*it).m_event2
							<< " duplicated!" << endl;
				}
				continue;
			}

			if ((*it).m_coveredBy != -1) {
//				cout << "Race is covered by: " << (*it).m_coveredBy << " , skip"
//						<< endl;
				continue;//if this race is covered by some other races, then skip.
			} else {
//				cout << "Race is Not covered" << endl;
			}

//			if (var_name.find("[JSActivation") != std::string::npos) {
//				non_replayable++;
//				continue;
//			}

			cout << "-------------------New Race Info: " << ii << "/"
					<< race_size << "---------------------------" << endl;

			cout << "\n\n--Race Info for latest replay" << endl;
			cout << "--Var is:  " << (*it).m_varId << " : " << var_name << endl;
			cout << (*it).m_event1 << "  VS  " << (*it).m_event2 << endl;
			cout << endl;
			if (var_name.find("readystatechange") != std::string::npos) {
				ajax_race++;
				continue;
			}
			if (var_name.find("Timer:") != std::string::npos) {
				sync_race++;
				continue;
			}
			if ((*it).m_event2 <= 17 || (*it).m_event1 <= 17) {
				bogus_race++;
				continue;
			}

			if (var_name.find("JSActivation[") != std::string::npos) {
				closure_race++;
				continue;
			}

			VarsInfo::AllVarData::const_iterator var_it =
					m_vinfo.variables().find((*it).m_varId);
			const VarsInfo::VarData& var_data = var_it->second;
			const VarsInfo::VarAccess* loc1 = var_data.findAccessLocation(
					(*it).m_access1 == VarsInfo::MEMORY_READ, (*it).m_event1);
			const VarsInfo::VarAccess* loc2 = var_data.findAccessLocation(
					(*it).m_access2 == VarsInfo::MEMORY_READ, (*it).m_event2);

			if (loc1 != NULL && loc2 != NULL) {
//			cout << "--Event1:  " << (*it).m_event1 << endl;
				std::string s1, s2;
				std::string ss1, ss2;
				printVarAccessCallTrace(*loc1,
						StringPrintf("%s <b>%s</b>",
								VarsInfo::Race_Info::AccessStr((*it).m_access1),
								HTMLEscape(var_name).c_str()), &s1);
				m_actionPrinter->printEventActionDetails((*it).m_event1, &ss1);
//				cout << "--Event1:  " << (*it).m_event1 << "\n" << s1 << endl;
//				cout << "--Full Event1:  " << (*it).m_event1 << "\n" << ss1
//						<< endl;
				cout << "------------------------------------------------"
						<< endl;

//			cout << "--OP1: " << s << endl;

				vector<string> op1 = RaceHandler::Instance()->split_string(s1,
						"***");

//			for (vector<string>::iterator itOp1 = op1.begin();
//					itOp1 != op1.end(); ++itOp1) {
//				cout << "---" << (*itOp1) << endl;
//			}
				pair<string, pair<string, string> > RaceInformation1;
				if (RaceHandler::Instance()->op_to_raceInfo.find((*it).m_event1)
						!= RaceHandler::Instance()->op_to_raceInfo.end()) {
					RaceInformation1 =
							RaceHandler::Instance()->op_to_raceInfo[(*it).m_event1];
				} else {
					RaceInformation1 = getRaceInfoFromString(&op1,
							(*it).m_event1, ss1);
					///lu: don't record event since for same eit, it might be differnet, for bubble events
//					RaceHandler::Instance()->op_to_raceInfo[(*it).m_event1] =
//							RaceInformation1;
				}

//			cout << "\n--Event2:  " << (*it).m_event2 << endl;

//				cout << "\n-----------end of event 1---------------\n" << endl;

				printVarAccessCallTrace(*loc2,
						StringPrintf("%s <b>%s</b>",
								VarsInfo::Race_Info::AccessStr((*it).m_access2),
								HTMLEscape(var_name).c_str()), &s2);

				m_actionPrinter->printEventActionDetails((*it).m_event2, &ss2);
//				cout << "--Event2:  " << (*it).m_event2 << "\n" << s2 << endl;

//				cout << "--Full Event2:  " << (*it).m_event2 << "\n" << ss2
//						<< endl;

				cout << "------------------------------------------------"
						<< endl;
//			cout << "--OP2: " << s << endl;
				vector<string> op2 = RaceHandler::Instance()->split_string(s2,
						"***");
//			for (vector<string>::iterator itOp2 = op2.begin();
//					itOp2 != op2.end(); ++itOp2) {
//				cout << "---" << (*itOp2) << endl;
//			}
//			pair<string, pair<string, string> > RaceInformation2 = getRaceInfo(
//					&op2, (*it).m_event2);

				pair<string, pair<string, string> > RaceInformation2;
				if (RaceHandler::Instance()->op_to_raceInfo.find((*it).m_event2)
						!= RaceHandler::Instance()->op_to_raceInfo.end()) {
					RaceInformation2 =
							RaceHandler::Instance()->op_to_raceInfo[(*it).m_event2];
				} else {
					RaceInformation2 = getRaceInfoFromString(&op2,
							(*it).m_event2, ss2);
					///lu: don't record event since for same eit, it might be differnet, for bubble events
//					RaceHandler::Instance()->op_to_raceInfo[(*it).m_event2] =
//							RaceInformation2;
				}

				if (RaceInformation1.first.compare("NULL") != 0
						&& RaceInformation2.first.compare("NULL") != 0) {

					cout << "Event 1 Id: " << RaceInformation1.second.first
							<< endl;
					cout << "Event 1 Tp: " << RaceInformation1.second.second
							<< endl;

					cout << "Event 2 Id: " << RaceInformation2.second.first
							<< endl;
					cout << "Event 2 Tp: " << RaceInformation2.second.second
							<< endl;

					bool toContinue = false;

					//DOMContentLoaded already handled
					if (RaceInformation1.second.first.empty()) {
						if (RaceInformation1.second.second.compare(
								"onDOMContentLoaded") == 0) {
							RaceInformation1.second.first = "Lu_DOM";
						} else {
							no_info++;
							cout<<"no info race"<<endl;
//							toContinue = true;
							continue;
						}
					} else if (RaceInformation2.second.first.empty()) {
						if (RaceInformation2.second.second.compare(
								"onDOMContentLoaded") == 0) {
							RaceInformation2.second.first = "Lu_DOM";
						} else {
							no_info++;
							cout<<"no info race"<<endl;
//							toContinue = true;
							continue;
						}
					}

					if (RaceInformation1.second.second.empty()) {
//						toContinue = true;
						continue;
					} else if (RaceInformation2.second.second.empty()) {
//						toContinue = true;
						continue;
					}

					//filter ajax for now
					if (RaceInformation1.second.second.compare(
							"onreadystatechange") == 0) {
//						toContinue = true;
						ajax_race++;
						continue;
					} else if (RaceInformation2.second.second.compare(
							"onreadystatechange") == 0) {
//						toContinue = true;
						ajax_race++;
						continue;
					}

					if (RaceInformation1.second.second.find("__BUBBLE")
							!= string::npos
							|| RaceInformation1.second.second.find("__CAPTURE")
									!= string::npos) {
//						toContinue = true;
						bubble_race++;
						continue;
					} else if (RaceInformation2.second.second.find("__BUBBLE")
							!= string::npos
							|| RaceInformation2.second.second.find("__CAPTURE")
									!= string::npos) {
//						toContinue = true;
						bubble_race++;
						continue;
					}

					string Event1String = "" + RaceInformation1.second.first
							+ "__" + RaceInformation1.second.second;

					string Event2String = "" + RaceInformation2.second.first
							+ "__" + RaceInformation2.second.second;

					// check bogus race

					cout << "e1: " << Event1String << endl;
					cout << "e2: " << Event2String << endl;

					if (checkSpecialEventHandler(Event1String, Event2String)) {
						bogus_race++;
//						toContinue = true;
						cout << "Bogus race!" << endl;
						continue;
					} else if (checkSpecialEventHandler(Event2String,
							Event1String)) {
						bogus_race++;
//						toContinue = true;
						cout << "Bogus race!" << endl;
						continue;
					}

					if (toContinue) {
//						cout << "Not replayable!" << endl;
						continue;
					}

//					if(Event1String.find("onmouse")!= string::npos && Event2String.find("onmouse")!= string::npos){
//
//					}

//					if (var_name.find("Lu_DOM__onclick") != std::string::npos) {
//						bubble_race++;
//						continue;
//					}

//					cout<<"Var name : "<<var_name<<endl;

					//if both of the two events are replayable

					pair<int, int> curr_race = make_pair((*it).m_event1,
							(*it).m_event2);
					if (existing_races.find(curr_race)
							!= existing_races.end()) {
						if ((*it).m_coveredBy == -1) {
							duplicated++;
							cout << (*it).m_event1 << " -> " << (*it).m_event2
									<< " duplicated!" << endl;
						}
						continue;
					} else {
						existing_races[curr_race] = 1;
					}

					if (RaceInformation1.second.first.empty()
							|| RaceInformation2.second.first.empty()
							|| RaceInformation1.second.second.empty()
							|| RaceInformation2.second.second.empty()) {
						no_info++;
						cout<<"no info race"<<endl;
						continue;

					}

					++replayableRacesCount;
					string raceTemplete =
							RaceHandler::Instance()->readRaceTemplete(
									"raceTemplete.txt");
					string raceTemplete_B = raceTemplete;
					//e.g., race1a

					stringstream ss;
					ss << replayableRacesCount;

					//					cout<<"Var name : "<<var_name<<endl;
					RaceHandler::Instance()->str_replace(raceTemplete,
							"@raceVar@", var_name);
					{
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@raceId@", ss.str());
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@raceAorB@", "a");
						//event 1 id
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@event1Id@",
								RaceInformation1.second.first.c_str());
						//event 1 type
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@event1Type@",
								RaceInformation1.second.second.c_str());
						//event 2 id
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@event2Id@",
								RaceInformation2.second.first.c_str());
						//event 2 type
						RaceHandler::Instance()->str_replace(raceTemplete,
								"@event2Type@",
								RaceInformation2.second.second.c_str());
					}
					RaceHandler::Instance()->str_replace(raceTemplete_B,
							"@raceVar@", var_name);
					{
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@raceId@", ss.str());
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@raceAorB@", "b");
						//event 2 id
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@event2Id@",
								RaceInformation1.second.first.c_str());
						//event 2 type
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@event2Type@",
								RaceInformation1.second.second.c_str());
						//event 1 id
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@event1Id@",
								RaceInformation2.second.first.c_str());
						//event 1 type
						RaceHandler::Instance()->str_replace(raceTemplete_B,
								"@event1Type@",
								RaceInformation2.second.second.c_str());
					}
					cout << raceTemplete << endl;

					cout << raceTemplete_B << endl;

					char fileA[50];
					sprintf(fileA, "race%d%c.txt", replayableRacesCount, 'a');

					char fileB[50];
					sprintf(fileB, "race%d%c.txt", replayableRacesCount, 'b');

					RaceHandler::Instance()->logToRace(fileA, raceTemplete);
					RaceHandler::Instance()->logToRace(fileB, raceTemplete_B);

				}

				else {
					vector<string>::iterator it_op;

					bool to_break = false;

					for (it_op = op1.begin(); it_op != op1.end(); ++it_op) {
						if ((*it_op).find("event:") != string::npos) {
							no_info++;
							to_break = true;
							cout<<"no info race"<<endl;
							break;
						}
					}

					if (to_break == true)
						continue;

					for (it_op = op2.begin(); it_op != op2.end(); ++it_op) {
						if ((*it_op).find("event:") != string::npos) {
							no_info++;
							to_break = true;
							cout<<"no info race"<<endl;
							break;

						}
					}
					if (to_break == true)
						continue;
					cout << "Not replayable!" << endl;
				}

//				cout << "\n-----------end of event 1---------------\n" << endl;
			}

		}
	}

//	int uncovered = 0;
//	int duplicated = 0;
//	int non_replayable = 0;

	cout << "Final uncovered    :   " << uncovered << endl;
	cout << "Final replayable   :   " << replayableRacesCount << endl;
	cout << "Final non-replayble:   " << non_replayable << endl;
	cout << "Final ajax_race    :   " << ajax_race << endl;
	cout << "Final sync_race    :   " << sync_race << endl;
	cout << "Final timer_race   :   " << timer_race << endl;
	cout << "Final bogus_race   :   " << bogus_race << endl;
	cout << "Final closure_race :   " << closure_race << endl;
	cout << "Final bubble_race  :   " << bubble_race << endl;
	cout << "Final no_info      :   " << no_info << endl;

	cout << "Final unhandled    :   "
			<< uncovered - replayableRacesCount - sync_race - timer_race
					 - bogus_race - closure_race
					- bubble_race - ajax_race - no_info << endl;
	cout << "\nFiltered Duplicated   :   " << duplicated - count_pre_run
			<< endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
//	for (ActionLog::EventActionSet::iterator I =
//			m_actions.m_eventActions.begin(), E =
//			m_actions.m_eventActions.end(); I != E; ++I) {
//
////		printf("-------------Event: %d------------------------\n", I->first);
//
//		std::string temp;
//		m_actionPrinter->printEventActionDetails(I->first, &temp);
////		printf("%s\n\n", temp.c_str());
//
//	}

//	exit(0);
//}

bool RaceApp::checkSpecialEventHandler(string e1, string e2) {
	//Lu_DOM__onDOMContentLoaded
	if (e1.compare("Lu_DOM__onDOMContentLoaded") == 0
			|| e1.compare("Lu_DOM__onload") == 0) {
		if (e2.find("__parsed") != string::npos) {
			return true;
		}
	}
	if (e1.compare("Lu_window__onload") == 0) {
		if (e2.find("Lu_DOM__onDOMContentLoaded") != string::npos) {
			return true;
		}
		if (e2.find("__parsed") != string::npos) {
			return true;
		}
		if (e2.find("__onload") != string::npos) {
			return true;
		}
		if ((e2.find("_script_") != string::npos
				|| e2.find("_src_") != string::npos)
				&& e2.find("__onclick") != string::npos) {
			return true;
		}
	}

	return false;
}

pair<string, pair<string, string> > RaceApp::getRaceInfoFromString(
		vector<string> *ops, int eid, string fullEvent) {

	cout << "check eid: " << eid << endl;

	string isFiringEvent_ = isFiringEvent(ops);
	string isParsingDOM_ = isParsingDOM(ops);
//	string isParsingScript_ = isParsingScript(ops);
	string isNetWorkEvent_ = isNetWorkEvent(ops);
	string isTimerEvent_ = isTimerEvent(ops);
	string isScriptRunner_ = isScriptRunner(ops);

	if (isFiringEvent_.empty() && isParsingDOM_.empty()
			&& isNetWorkEvent_.empty() && isScriptRunner_.empty()) {
		vector<string> opFull = RaceHandler::Instance()->split_string(fullEvent,
				"***");
		isFiringEvent_ = isFiringEvent(&opFull);
		isParsingDOM_ = isParsingDOM(&opFull);
		isNetWorkEvent_ = isNetWorkEvent(&opFull);
		isScriptRunner_ = isScriptRunner(&opFull);
		isTimerEvent_ = isTimerEvent(&opFull);
	}

	if (!isFiringEvent_.empty()) {

//		if (isFiringEvent_.find("BUBBLE") != string::npos) {
//			cout << "Ignore bubble race!!" << endl;
//			bubble_race++;
//			return make_pair("NULL", make_pair("NULL", "NULL"));
//		}

		cout << "--It's a firing event: " << isFiringEvent_ << endl;
		string type_prt = RaceHandler::Instance()->split_string(isFiringEvent_,
				":")[1];
		type_prt = RaceHandler::Instance()->split_string(type_prt, ",")[0];

		string type = RaceHandler::Instance()->split_string(type_prt, "@")[0];
		string ptr = RaceHandler::Instance()->split_string(type_prt, "@")[1];

		type = RaceHandler::Instance()->trimString(type);
		ptr = RaceHandler::Instance()->trimString(ptr);

		string id = RaceHandler::Instance()->ptr_id[ptr];

		cout << "--Type: " << type << " , ptr: " << ptr << " , id: " << id
				<< endl;

		if (type.compare("load") == 0 && id.empty()) {
			id = "Lu_window";
		}

		if (isFiringEvent_.find("BUBBLE") != string::npos
				|| isFiringEvent_.find("CAPTURE") != string::npos) {
			cout << "Ignore bubble race!!" << endl;
			return make_pair("Firing", make_pair(id, "on" + type + "__BUBBLE"));
		}

		return make_pair("Firing", make_pair(id, "on" + type));

	} else if (!isParsingDOM_.empty()) {

//		cout << eid<<"--It's a parsing event: " << isParsingDOM_;
		string url = isParsingDOM_;
		url = url.substr(11);
		url = url.substr(0, url.length() - 4);
		cout << eid << " : -It's a parsing event: \n" << url << endl;
		string id = "";
		if (url.compare(RaceHandler::Instance()->originalHTML) != 0) {
			id = "Src:::" + url;
		}

		else {
			id = RaceHandler::Instance()->getWholeElement(eid);
		}

		if (!id.empty()) {

			std::string prefix = "Src:::file://";
			if (id.substr(0, prefix.size()) == prefix) {
				return make_pair("Parsing", make_pair(id + "_ID", id + "_TYPE"));
			}

			cout << "--id  : " << id << endl;
			return make_pair("Parsing", make_pair(id, id + "__parsed"));
		}
//	} else if (!isParsingScript_.empty()) {

	} else if (!isNetWorkEvent_.empty()) {

	} else if (!isTimerEvent_.empty()) {

		cout << "Timer event detected !!!" << isTimerEvent_ << endl;
		timer_race++;
	}

	else if (!isScriptRunner_.empty()) {

//		cout << fullEvent << endl;

		vector<string> op2 = RaceHandler::Instance()->split_string(fullEvent,
				"***");

		bool canHandle = false;

		for (vector<string>::iterator it = op2.begin(); it != op2.end(); ++it) {

			string tmp = (*it);
//			cout << ">>" << tmp << endl;

			if (tmp.compare("declare_jsfunction") == 0) {
				canHandle = true;
			}

			if (tmp.length() > 3 && tmp.substr(0, 3).compare("JS[") == 0) {
				vector<string> op3 = RaceHandler::Instance()->split_string(tmp,
						"]:");
				if (op3.size() == 2) {

					if (op3[1].find("GlobalCode") == 0) {
						string temp1 = op3[0];

						temp1 = temp1.substr(3, temp1.length() - 3);

						vector<string> op4 =
								RaceHandler::Instance()->split_string(temp1,
										"/");

						temp1 = op4[op4.size() - 1];

//						std::replace(temp1.begin(), temp1.end(), ':', '_');
//						std::replace(temp1.begin(), temp1.end(), '/', '_');
//						std::replace(temp1.begin(), temp1.end(), '.', '_');

//						cout << temp1 << endl;

						return make_pair("ScriptRunner",
								make_pair("_script_" + temp1, "onclick"));

					}
				}

			}

		}

	}

	return make_pair("NULL", make_pair("NULL", "NULL"));
}

string RaceApp::isFiringEvent(vector<string> *ops) {
	string retVal = "";

	for (vector<string>::iterator itOp = ops->begin(); itOp != ops->end();
			++itOp) {
//		cout << "---" << (*itOp) << endl;
		if (!(*itOp).empty() && (*itOp).find("Fire Event:") != string::npos) {
			cout << "--->" << (*itOp) << endl;
			return (*itOp);
		}
	}

	return retVal;
}

string RaceApp::isTimerEvent(vector<string> *ops) {
	string retVal = "";

//	if (!(*itOp).empty() && (*itOp).find("read <b>Timer:") != string::npos)
//	if (!(*itOp).empty() && (*itOp).find("value <b>DOMTimer") != string::npos)
//	if (!(*itOp).empty() && (*itOp).find("[[function:") != string::npos)

	string temp1 = "";
	string temp2 = "";
	string temp3 = "";

	for (vector<string>::iterator itOp = ops->begin(); itOp != ops->end();
			++itOp) {
//		cout << "---" << (*itOp) << endl;
		if (!(*itOp).empty() && (*itOp).find("read <b>Timer:") != string::npos
				&& temp1.empty()) {
			temp1 = (*itOp);
		}

		if (!(*itOp).empty()
				&& (*itOp).find("value <b>DOMTimer") != string::npos
				&& temp2.empty()) {
			temp2 = (*itOp);
		}

		if (!(*itOp).empty() && (*itOp).find("[[function:") != string::npos
				&& temp3.empty()) {
			temp3 = (*itOp);
		}

		if (!temp1.empty() && !temp2.empty() && !temp3.empty()) {
//			cout << "---" << temp1 << endl;
//			cout << "---" << temp2 << endl;
//			cout << "---" << temp3 << endl;
			return temp3;
		}

	}

	return retVal;
}

string RaceApp::isParsingDOM(vector<string> *ops) {
	string retVal = "";
	for (vector<string>::iterator itOp = ops->begin(); itOp != ops->end();
			++itOp) {
//		cout << "---" << (*itOp) << endl;

		if (!(*itOp).empty() && (*itOp).find("parse HTML[") != string::npos) {
			return (*itOp);
		}
	}

	return retVal;
}

string RaceApp::isScriptRunner(vector<string> *ops) {
	string retVal = "";

	for (vector<string>::iterator itOp = ops->begin(); itOp != ops->end();
			++itOp) {
//		cout << "---" << (*itOp) << endl;

		if (!(*itOp).empty()
				&& (*itOp).find("script runner timer") != string::npos) {
			return (*itOp);
		}
	}

	return retVal;
}

string RaceApp::isParsingScript(vector<string> *ops) {
	string retVal = "";

	for (vector<string>::iterator itOp = ops->begin(); itOp != ops->end();
			++itOp) {
//		cout << "---" << (*itOp) << endl;
	}

	return retVal;
}

string RaceApp::isNetWorkEvent(vector<string> *ops) {
	string retVal = "";

	for (vector<string>::iterator itOp = ops->begin(); itOp != ops->end();
			++itOp) {
//		cout << "---" << (*itOp) << endl;
	}

	return retVal;
}

RaceApp::~RaceApp() {
	delete m_actionPrinter;
}

////
////
////
////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
////
////  HTML Page Handlers
////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// Handler for /
void RaceApp::handleInfo(const std::string& params, std::string* response) {
	addHeader(response, "Info");
	StringAppendF(response,
			"<div style=\"width: 500px\"><h3>Welcome to EventRacer.</h3>"
					"<p>The input file %s was processed.</p>\n"
					"<p><a href=\"varlist\">Click <b>here</b> for a list of memory locations with uncovered races</a></p>\n"

					"<h3>Details.</h3>"
					"<p>The input contains %d memory locations. "
					"You list the memory locations with uncovered races by clicking <a href=\"varlist\"><b>here</b></a></p>\n"

					"<p>Alternatively, one can explore the <a href=\"hb\">happens-before graph</a> and view "
					"the recorded <a href=\"code\">operations</a> in every event action (node in the graph). Note: the "
					"happens-before graph has %d nodes and we display only part of the graph. To explore other parts "
					"of the graph, click on the nodes.</p>"
					""
					"<p>Finally, one can search by memory location name.</p></div>",
			HTMLEscape(m_fileName).c_str(), m_vars.numEntries(),
			m_actions.maxEventActionId());
	displaySearchBox("", 0, response);

	addFooter(response);
}

// Handler for /varlist
void RaceApp::handleVarList(const std::string& params, std::string* response) {
	addHeader(response, "Memory Locations");
	URLParams p;
	p.parse(params);

	int filter_level = p.getIntDefault("filter_level", 3);
	std::string var_name = p.getString("varname");
	displaySearchBox(var_name, filter_level, response);

	response->append("Shown memory locations: ");
	{
		for (int i = 0; i < 6; ++i) {
			if (i == filter_level) {
				response->append("<b>");
			}
			URLParams p1 = p;
			p1.setInt("filter_level", i);
			StringAppendF(response, "<a href=\"varlist?%s\">",
					p1.toString().c_str());  // p1.toString() URL escapes.
			switch (i) {
			case 0:
				response->append("[all]");
				break;
			case 1:
				response->append("");
				break;
			case 2:
				response->append("[only with races]");
				break;
			case 3:
				response->append("[only with uncovered races]");
				break;
			case 4:
				response->append("[only with uncovered unfiltered races]");
				break;
			case 5:
				response->append("[only with high risk races]");
				break;
			}
			response->append("</a>&nbsp;&nbsp;");
			if (i == filter_level) {
				response->append("</b>");
			}
		}
	}

	enum Columns {
		VAR_TYPE = 0, VAR_NAME, NUM_RACES, NUM_UNCOVERED_RACES, TAGS, NUM_COLS
	};

	HTMLTable::addJavaScript(response);

	HTMLTable table(NUM_COLS, response);

	table.setColumn(VAR_TYPE, "Type");
	table.setColumn(VAR_NAME, "Name");
	table.setColumn(NUM_RACES, "Num. races");
	table.setColumn(NUM_UNCOVERED_RACES, "Num. uncovered races");
	table.setColumn(TAGS, "Race filters");
	table.writeHeader();

	for (int level = 5; level >= filter_level; --level) {
		for (VarsInfo::AllVarData::const_iterator it =
				m_vinfo.variables().begin(); it != m_vinfo.variables().end();
				++it) {
			int var_id = it->first;

			// Restrict variables by the search term.
			if (!var_name.empty() && strstr(m_vars.getString(var_id), var_name.c_str()) == NULL) {continue;
		}

			const VarsInfo::VarData& data = it->second;

			if (getVarFilterLevel(var_id, data) != level)
				continue;

			table.setColumn(VAR_TYPE,
					VarTypeByName(m_vars.getString(it->first)));
			table.setColumn(VAR_NAME,
					HTMLEscape(ShortenStr(m_vars.getString(it->first), 64)));
			cout << HTMLEscape(ShortenStr(m_vars.getString(it->first), 64))
					<< endl;
			table.setColumnF(NUM_RACES, "%d",
					static_cast<int>(data.m_allRaces.size()));
			table.setColumnF(NUM_UNCOVERED_RACES, "%d",
					static_cast<int>(data.m_noParentRaces.size()));
			table.setColumn(TAGS, getVarTagsString(var_id));
			cout << getVarTagsString(var_id) << endl;

			std::string extra;
			StringAppendF(&extra,
					"<b>Uncovered races:</b> (click race ids for details) %s<br>",
					raceSetStr(data.m_noParentRaces).c_str());
			{
				std::vector<int> covered_races;
				std::set_difference(data.m_allRaces.begin(),
						data.m_allRaces.end(), data.m_noParentRaces.begin(),
						data.m_noParentRaces.end(),
						std::back_inserter(covered_races));
				StringAppendF(&extra, "<b>Covered races:</b> %s %s<br>",
						raceSetStr(covered_races).c_str(),
						data.m_parentRaces.empty() ?
								"" :
								StringPrintf("(covered by %s)",
										getRaceVars(data.m_parentRaces).c_str()).c_str());
			}
			StringAppendF(&extra,
					"<b>Values occurring in the trace:</b> %s<br>",
					HTMLEscape(m_raceTags.getVarDefSet(it->first)).c_str());
			StringAppendF(&extra,
					"List all <a href=\"var?id=%d\" title=\"%s\">event actions</a> with reads and writes of variable<br>",
					it->first, HTMLEscape(m_vars.getString(it->first)).c_str());

			table.writeExpandableRow(
					level == 3 ? "k" : (level == 5 ? "s" : "u"), extra);
		}
	}

	table.writeFooter(true);

	addFooter(response);
}

// Handler for /var
void RaceApp::handleVarDetails(const std::string& params,
		std::string* response) {
	URLParams p;
	p.parse(params);
	int var_id = p.getIntDefault("id", 0);
	VarsInfo::AllVarData::const_iterator var_it = m_vinfo.variables().find(
			var_id);
	if (var_it == m_vinfo.variables().end()) {
		response->append("<html><body>Unknown variable</body></html>");
		return;
	}
	const VarsInfo::VarData& data = var_it->second;
	string var_name = m_vars.getString(var_id);
	addHeader(response, var_name);

	StringAppendF(response,
			"<h2>List of reads and writes of %s in their trace order.</h2>",
			HTMLEscape(var_name).c_str());

	HTMLTable::addJavaScript(response);

	HTMLTable table(3, response);
	table.setColumn(0, "Event actions");
	table.setColumn(1, "Uncovered races");
	table.setColumn(2, "Covered races");
	table.writeHeader();

	std::string card;
	std::string traces;
	std::vector<int> uncovered_races;
	std::vector<int> covered_races;

	int last_event_action_id = -1;
	for (size_t access_i = 0; access_i <= data.m_accesses.size(); ++access_i) {
		if (last_event_action_id != -1
				&& (access_i == data.m_accesses.size()
						|| last_event_action_id
								!= data.m_accesses[access_i].m_eventActionId)) {
			table.setColumnF(0,
					"Event action %s<br><pre class=\"blue\">%s\n</pre>",
					eventActionAsStr(last_event_action_id).c_str(),
					card.c_str());
			table.setColumn(1, raceSetStr(uncovered_races));
			table.setColumn(2, raceSetStr(covered_races));

			table.writeExpandableRow("u", traces);

			card.clear();
			traces.clear();
			uncovered_races.clear();
			covered_races.clear();
		}
		if (access_i == data.m_accesses.size())
			break;

		const VarsInfo::VarAccess& access = data.m_accesses[access_i];
		last_event_action_id = access.m_eventActionId;
		bool is_read = access.m_isRead;

		// Display the type of the event action.
		if (card.empty()) {
			std::vector<int> call_trace;
			m_callTraceBuilder.getCallTraceOfCommand(access.m_eventActionId,
					access.m_commandIdInEvent, &call_trace);
			if (!call_trace.empty()) {
				const ActionLog::EventAction& event = m_actions.event_action(
						access.m_eventActionId);
				StringAppendF(&card, "  %s\n   ...\n",
						HTMLEscape(
								m_scopes.getString(
										event.m_commands[call_trace[0]].m_location)).c_str());
			}
		} else {
			card.append("   ...\n");
		}

		// Display the read/written values.
		std::string value;
		if (getAccessValue(access.m_eventActionId, access.m_commandIdInEvent,
				&value)) {
			StringAppendF(&card, "    %s <b>%s</b>\n",
					is_read ? "Read value" : "Write value",
					HTMLEscape(value).c_str());
		} else {
			StringAppendF(&card, "    %s\n", is_read ? "Read" : "Write");
		}

		// List races for the current event action.
		for (size_t i = 0; i < data.m_allRaces.size(); ++i) {
			int race_id = data.m_allRaces[i];
			const VarsInfo::Race_Info& race = m_vinfo.races()[race_id];
			if ((race.m_event1 == access.m_eventActionId
					&& race.m_cmdInEvent1 == access.m_commandIdInEvent)
					|| (race.m_event2 == access.m_eventActionId
							&& race.m_cmdInEvent2 == access.m_commandIdInEvent)) {
				if (race.m_coveredBy == -1 && race.m_multiParentRaces.empty()) {
					uncovered_races.push_back(race_id);
				} else {
					covered_races.push_back(race_id);
				}
			}
		}

		// Show call trace if the row is expanded.
//		cout << "3: " << traces << endl;
		StringAppendF(&traces,
				"<h4>Call trace of a %s %s in event action %d</h4>"
						"<p>Only the first %s in a event action is recorded.</p>",
				is_read ? "read from" : "write to",
				HTMLEscape(var_name).c_str(), access.m_eventActionId,
				is_read ? "read" : "write");
//		cout<<"--"<<HTMLEscape(var_name).c_str()<<endl;
//		cout<<"--"<<access.m_eventActionId<<endl;
//		cout << "3: " << traces << endl;
		printVarAccessCallTrace(access,
				StringPrintf("%s <b>%s</b>", is_read ? "Read" : "Write",
						HTMLEscape(var_name).c_str()), &traces);
//		cout << "******************************" << endl;
//		cout << "1: " << HTMLEscape(var_name).c_str() << endl;
//		cout << "2: " << access.m_eventActionId << endl;
//		cout << "3: " << traces << endl;
//		cout << "4: -------------------------------------------------" << endl;
	}

	table.writeFooter(true);

	addFooter(response);
}

// Handler for /race
void RaceApp::handleRaceDetails(const std::string& params,
		std::string* response) {
	URLParams p;
	p.parse(params);
	int race_id = p.getIntDefault("id", 0);
	if (race_id < 0 || static_cast<size_t>(race_id) >= m_vinfo.races().size()) {
		response->append("<html><body>Unknown race</body></html>");
		return;
	}
	const VarsInfo::Race_Info& race = m_vinfo.races()[race_id];
	VarsInfo::AllVarData::const_iterator var_it = m_vinfo.variables().find(
			race.m_varId);
	if (var_it == m_vinfo.variables().end()) {
		response->append("<html><body>Unknown variable</body></html>");
		return;
	}
	const VarsInfo::VarData& var_data = var_it->second;
	string var_name = m_vars.getString(race.m_varId);

	addHeader(response,
			StringPrintf("Race #%d on %s", race_id,
					HTMLEscape(var_name).c_str()));

	showRaceInfo(race_id, response);

	response->append(
			"<ul><li>A race is a pair of operations <i>op1</i> and <i>op2</i> such that in our trace we observe "
					"them in the order <i>op1</i>, <i>op2</i>, but they are unordered accoriding to the happens-before relation.");
	if (race.m_coveredBy == -1 && race.m_multiParentRaces.empty()) {
		response->append(
				"<li>This is an <b>uncovered race</b>. This means that there exists an execution, for which "
						"<i>op2</i> executes without <i>op1</i> before it.");
	}
	response->append("</ul>");

	const VarsInfo::VarAccess* loc1 = var_data.findAccessLocation(
			race.m_access1 == VarsInfo::MEMORY_READ, race.m_event1);
	const VarsInfo::VarAccess* loc2 = var_data.findAccessLocation(
			race.m_access2 == VarsInfo::MEMORY_READ, race.m_event2);
	if (loc1 != NULL && loc2 != NULL) {
		HTMLTable::addJavaScript(response);

		HTMLTable table(2, response);
		table.setColumn(0, "Op");
		table.setColumn(1, "Call trace of the operation");
		table.writeHeader();

		table.setColumn(0, "<i>op1</i>");
		std::string s;
		printVarAccessCallTrace(*loc1,
				StringPrintf("%s <b>%s</b>",
						VarsInfo::Race_Info::AccessStr(race.m_access1),
						HTMLEscape(var_name).c_str()), &s);
		table.setColumn(1, "<br>" + s + "<br>");
		table.writeRow("u");

		table.setColumn(0, "<i>op2</i>");
		s.clear();
		printVarAccessCallTrace(*loc2,
				StringPrintf("%s <b>%s</b>",
						VarsInfo::Race_Info::AccessStr(race.m_access2),
						HTMLEscape(var_name).c_str()), &s);
		table.setColumn(1, "<br>" + s + "<br>");
		table.writeRow("u");

		table.writeFooter(false);
	}

	response->append(
			"<h2>Summary of the happens-before graph with the race</h2>"
					"<ul><li>This race is in red. Other races are in green.");
	if (race.m_coveredBy != -1) {
		const VarsInfo::Race_Info& parent_race =
				m_vinfo.races()[race.m_coveredBy];
		if (parent_race.m_event1 == race.m_event1
				&& parent_race.m_event2 == race.m_event2) {
			response->append(
					"<li>This race is covered by another race in the same event actions.</li>");
		} else {
			response->append(
					"<li>Races that cover this race are in blue.</li>");
		}
	}
	response->append("</ul>");

	// Display a graph between the event actions
	int focus_id = p.getIntDefault("focus", -1);
	EventGraphDisplay display_events("race",
			StringPrintf("race%llu_%u_%u", m_appId, race_id, focus_id), p,
			&m_actions, &m_graphInfo, &m_inputEventGraph, &m_graphWithTimers);

	display_events.tryIncludeNode(
			m_callTraceBuilder.eventCreatedBy(race.m_event1),
			EventGraphDisplay::NODE_FOCUS_CAUSE, "trigger_op1");
	display_events.tryIncludeNode(
			m_callTraceBuilder.eventCreatedBy(race.m_event2),
			EventGraphDisplay::NODE_FOCUS_CAUSE, "trigger_op2");

	displayRacesIfEnabled(p, &display_events);

	if (race.m_coveredBy != -1) {
		const VarsInfo::Race_Info& parent_race =
				m_vinfo.races()[race.m_coveredBy];
		display_events.tryIncludeNode(parent_race.m_event1,
				EventGraphDisplay::NODE_FOCUS_PARENT_RACE, "covered_by_op1");
		display_events.tryIncludeNode(parent_race.m_event2,
				EventGraphDisplay::NODE_FOCUS_PARENT_RACE, "covered_by_op2");
		display_events.addRaceArc(race.m_coveredBy, parent_race, "blue");
	}

	display_events.tryIncludeNode(race.m_event1,
			EventGraphDisplay::NODE_FOCUS_RACE, "op1");
	display_events.tryIncludeNode(race.m_event2,
			EventGraphDisplay::NODE_FOCUS_RACE, "op2");
	display_events.addRaceArc(race_id, race, "red");

	display_events.outputGraph(m_actionPrinter, response);
	addFooter(response);
}

// Handler for /js
void RaceApp::handleShowJS(const std::string& params, std::string* response) {
	// Displays a JavaScript code with a given id.
	URLParams p;
	p.parse(params);
	int jsid = p.getIntDefault("jsid", 0);
	std::string js(m_js.getString(jsid));
	addHeader(response, StringPrintf("Javascript #%d", jsid));
	response->append("<pre>");
	JsViewer jsviewer;
	jsviewer.jsToHTML(js, response);
	response->append("</pre>");
	addFooter(response);
}

// Handler for /code
void RaceApp::handleShowCode(const std::string& params, std::string* response) {
	// Displays the execution trace in one event action.
	URLParams p;
	p.parse(params);
	int event_action_id = p.getIntDefault("focus", 0);
	while (event_action_id < m_actions.maxEventActionId()
			&& m_actions.event_action(event_action_id).m_commands.size() == 0) {
		++event_action_id;
	}
	addHeader(response,
			StringPrintf("Execution trace in event action # %d",
					event_action_id));

	StringAppendF(response,
			"<p>Show event action in the <a href=\"hb?focus=%d\">happens-before graph</a>.</p>",
			event_action_id);

	// Write a table with the event action's predecessors and successors.
	HTMLTable::addJavaScript(response);
	HTMLTable table(1, response);
	table.writeHeader();
	std::string s;
	showEventsSummariesIntoTable(
			m_graphWithTimers.nodePredecessors(event_action_id), &s);
	table.setColumn(0, "Predecessor event actions (in HB graph)");
	table.writeExpandableRow("u", s);
	s.clear();
	showEventsSummariesIntoTable(
			m_graphWithTimers.nodeSuccessors(event_action_id), &s);
	table.setColumn(0, "Successor event actions (in HB graph)");
	table.writeExpandableRow("u", s);
	table.writeFooter(false);

	// Write the code of the event action.
	StringAppendF(response,
			"<h2>List of operations in %s event action %d</h2>\n",
			ActionLog::EventActionType_AsString(
					m_actions.event_action(event_action_id).m_type),
			event_action_id);
	m_actionPrinter->printEventActionDetails(event_action_id, response);

	addFooter(response);
}

// Handler for /hb
void RaceApp::handleBrowseGraph(const std::string& params,
		std::string* response) {
	URLParams p;
	p.parse(params);
	addHeader(response, "Happens before graph");
	int node_id = p.getIntDefault("focus", -1);
	if (node_id != -1) {
		StringAppendF(response,
				"<p>Highlighted event action %d [<a href=\"code?focus=%d\">see its execution trace</a>]</p>",
				node_id, node_id);
	}
	EventGraphDisplay graph_display("hb",
			StringPrintf("hb%llu_%u", m_appId, node_id), p, &m_actions,
			&m_graphInfo, &m_inputEventGraph, &m_graphWithTimers);
	displayRacesIfEnabled(p, &graph_display);
	graph_display.outputGraph(m_actionPrinter, response);
	addFooter(response);
}

// Handler for /undef
void RaceApp::handleUndefRaces(const std::string& params,
		std::string* response) {
	URLParams p;
	p.parse(params);
	int var_id = p.getIntDefault("var", 0);
	VarsInfo::AllVarData::const_iterator var_it = m_vinfo.variables().find(
			var_id);
	if (var_it == m_vinfo.variables().end()) {
		response->append("<html><body>Unknown variable</body></html>");
		return;
	}

	const VarsInfo::VarData& data = var_it->second;
	string var_name = m_vars.getString(var_id);
	addHeader(response,
			StringPrintf("Races with the first write to %s",
					HTMLEscape(m_vars.getString(var_id)).c_str()));

	if (data.numReads() == 0) {
		response->append("<h1>No reads</h1>");
	}
	if (data.numWrites() == 0) {
		response->append("<h1>No writes</h1>");
	} else {
		StringAppendF(response,
				"<p>This list includes all the reads from %s in relation to its first write. "
						"This is useful to look for reads that may read an uninitialized value. "
						"The developer must then manually inspect if all reads in uncovered races correctly"
						" handle undefined value.</p>",
				HTMLEscape(var_name).c_str());

		int node1 = data.getWriteWithIndex(0)->m_eventActionId;
		StringAppendF(response,
				"<h2>Initialization</h2><p>The variable is initialized (first written) in event action %d</p>",
				node1);

		printVarAccessCallTrace(*data.getWriteWithIndex(0),
				StringPrintf("Write <b>%s</b>", HTMLEscape(var_name).c_str()),
				response);

		StringAppendF(response,
				"<h2>List of reads in relation to initialization</h2>");

		HTMLTable::addJavaScript(response);

		HTMLTable table(2, response);
		table.setColumn(0, "Event Action");
		table.setColumn(1, "Ordering constraints with initialization");
		table.writeHeader();

		for (size_t i = 0; i < data.m_accesses.size(); ++i) {
			if (!data.m_accesses[i].m_isRead)
				continue;
			int node2 = data.m_accesses[i].m_eventActionId;
			int cmd_in_node2 = data.m_accesses[i].m_commandIdInEvent;

			table.setColumnF(0, "<a href=\"code?focus=%d\">%d</a>", node2,
					node2);

			bool node2before1 = node2 < node1;
			bool ordered = m_vinfo.fast_event_graph()->areOrdered(node1, node2);
			bool covered = false;
			if (!node2before1 && !ordered) {
				for (size_t j = 1; j < data.m_accesses.size(); ++j) {
					if (data.m_accesses[j].m_isRead)
						continue;
					std::vector<int> tmp;
					if (m_vinfo.hasPathViaRaces(
							data.m_accesses[j].m_eventActionId, node2,
							cmd_in_node2, &tmp)) {
						covered = true;
					}
				}
			}

			if (node2before1) {
				table.setColumnF(1,
						"A read that is before the initialization.");
			} else if (ordered) {
				table.setColumn(1,
						"A read that is ordered after the initialization.");
			} else if (covered) {
				table.setColumn(1,
						"Covered race: A read that may be ordered after the initialization.");
			} else {
				table.setColumn(1,
						"<b>Uncovered race:</b> A read that may happen before the initialization.");
			}

			std::string read_data;
			displayNodeRelation(node1, node2, cmd_in_node2, &read_data);
			printVarAccessCallTrace(data.m_accesses[i],
					StringPrintf("Read <b>%s</b>",
							HTMLEscape(var_name).c_str()), &read_data);
			table.writeExpandableRow(
					(node2before1 || ordered || covered) ? "u" : "s",
					read_data);
		}

		table.writeFooter(true);
	}

	addFooter(response);
}

// Handler for /child
void RaceApp::handleRaceChildren(const std::string& params,
		std::string* response) {
	URLParams p;
	p.parse(params);

	int64 start_time = GetCurrentTimeMicros();
	std::set<int> races;
	int var_id = p.getIntDefault("var", -1);
	int child_race_location = p.getIntDefault("child_loc", 0);
	if (var_id >= 0) {
		VarsInfo::AllVarData::const_iterator var_it = m_vinfo.variables().find(
				var_id);
		if (var_it == m_vinfo.variables().end()) {
			response->append("<html><body>Unknown variable</body></html>");
			return;
		}
		const VarsInfo::VarData& data = var_it->second;
		for (size_t i = 0; i < data.m_allRaces.size(); ++i) {
			m_vinfo.getDirectRaceChildren(data.m_allRaces[i],
					child_race_location != 0, &races);
		}
		addHeader(response,
				StringPrintf("Child races of %s",
						HTMLEscape(m_vars.getString(var_id)).c_str()));
	} else {
		int race_id = p.getIntDefault("race", -1);
		if (race_id < 0
				|| race_id >= static_cast<int>(m_vinfo.races().size())) {
			response->append(
					"<html><body>Please provide a valid var or race parameter</body></html>");
			return;
		}
		m_vinfo.getDirectRaceChildren(race_id, child_race_location != 0,
				&races);
		addHeader(response, StringPrintf("Child races of race %d", race_id));
	}
	printf("Done checking for direct child races (%lld ms)...\n",
			(GetCurrentTimeMicros() - start_time) / 1000);

	std::map<int, std::vector<int> > vars_and_races;
	for (std::set<int>::const_iterator it = races.begin(); it != races.end();
			++it) {
		vars_and_races[m_vinfo.races()[*it].m_varId].push_back(*it);
	}

	response->append("Child race location: ");
	{
		for (int i = 0; i < 2; ++i) {
			if (i == child_race_location) {
				response->append("<b>");
			}
			URLParams p1 = p;
			p1.setInt("child_loc", i);
			StringAppendF(response, "<a href=\"child?%s\">",
					p1.toString().c_str()); // URLParams.toString is URL encoded.
			switch (i) {
			case 0:
				response->append("[anywhere (var is not synchronizing)]");
				break;
			case 1:
				response->append(
						"[only later event actions (var is just lazy initialized)]");
				break;
			}
			response->append("</a>&nbsp;&nbsp;");
			if (i == child_race_location) {
				response->append("</b>");
			}
		}
	}

	response->append("<table><tr>"
			"<td>Tags</td>"
			"<td>Variable name</td>"
			"<td>Num Reads</td>"
			"<td>Num writes</td>"
			"<td>Child races</td>"
			"<td>Races</td>"
			"<td>Harm</td>"
			"<td>Def.Set.</td>"
			"</tr>");
	int num_rows = 0;
	for (std::map<int, std::vector<int> >::const_iterator it =
			vars_and_races.begin(); it != vars_and_races.end(); ++it) {
		const VarsInfo::VarData& data =
				m_vinfo.variables().find(it->first)->second;
		int num_reads = data.numReads();
		int num_writes = data.numWrites();
		RaceTags::RaceTagSet tags = m_raceTags.getVariableTags(it->first);
		bool with_undefined_init_race =
				m_raceTags.hasUndefinedInitilizationRace(it->first);
		++num_rows;
		StringAppendF(response, "<tr class=\"r%s%d\">"
				"<td>%s</td>"
				"<td><a href=\"var?id=%d\" title=\"%s\">%s</a></td>"
				"<td>%d</td>"
				"<td>%d</td>"
				"<td><a href=\"child?var=%d\">%s</a></td>"
				"<td>%s</td>"
				"<td><a href=\"undef?var=%d\">%s</a></td>"
				"<td>%s</td>"
				"</tr>", tags != RaceTags::emptyTagSet() ? "k" : "u",
				num_rows % 2, RaceTags::tagsToString(tags).c_str(), it->first,
				HTMLEscape(m_vars.getString(it->first)).c_str(),
				HTMLEscape(ShortenStr(m_vars.getString(it->first), 64)).c_str(),
				num_reads, num_writes, it->first,
				data.m_childRaces.size() == 0 ?
						"?" :
						StringPrintf("%d",
								static_cast<int>(data.m_childRaces.size())).c_str(),
				raceSetStr(it->second).c_str(), it->first,
				with_undefined_init_race ? "initialization race2" : "",
				HTMLEscape(m_raceTags.getVarDefSet(it->first)).c_str());
	}

	response->append("</table>");
	StringAppendF(response, "<p>%d rows</p>", num_rows);
	addFooter(response);
}

// Handler for /rel
void RaceApp::handleNodeRelation(const std::string& params,
		std::string* response) {
	// Displays the relation between a pair of nodes.
	URLParams p;
	p.parse(params);
	int node1 = p.getIntDefault("id1", -1);
	int node2 = p.getIntDefault("id2", -1);

	if (node1 < 0 || node2 < 0) {
		response->append("<html><body>Unknown nodes</body></html>");
		return;
	}

	addHeader(response,
			StringPrintf("Relation between %d and %d", node1, node2));
	displayNodeRelation(node1, node2, -1, response);
	addFooter(response);
}

////
////
////
////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
////
////  Utility functions
////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void RaceApp::displaySearchBox(const std::string& var_name, int filter_level,
		std::string* response) const {
	StringAppendF(response, "<form action=\"varlist\">\n"
			"  Search by name:"
			"  <input type=\"text\" name=\"varname\" value=\"%s\">\n"
			"  <input type=\"hidden\" name=\"filter_level\" value=\"%d\">\n"
			"  <input type=\"submit\" value=\"Search\">\n"
			"</form>\n", HTMLEscape(var_name).c_str(), filter_level);
}

std::string RaceApp::getRaceVars(const std::vector<int>& races) const {
	std::string result;
	std::set<int> vars;
	for (size_t i = 0; i < races.size(); ++i) {
		vars.insert(m_vinfo.races()[races[i]].m_varId);
	}
	for (std::set<int>::const_iterator it = vars.begin(); it != vars.end();
			++it) {
		if (it != vars.begin())
			result.append(" ");
		StringAppendF(&result, "<a href=\"var?id=%d\" title=\"%s\">%s</a>", *it,
				HTMLEscape(m_vars.getString(*it)).c_str(),
				HTMLEscape(ShortenStr(m_vars.getString(*it), 32)).c_str());
	}
	return result;
}

std::string RaceApp::raceSetStr(const std::vector<int>& races) const {
	std::string result;
	StringAppendF(&result, "%d", static_cast<int>(races.size()));
	if (!races.empty()) {
		result.append(" (");
		for (size_t i = 0; i < races.size(); ++i) {
			if (i != 0)
				result.append(" ");
			bool is_multi_covered =
					m_vinfo.races()[races[i]].m_multiParentRaces.size() != 0;
			if (is_multi_covered)
				result.append("<del>");
			StringAppendF(&result, "<a href=\"race?id=%d\">#%d</a>", races[i],
					races[i]);
			if (is_multi_covered)
				result.append("</del>");
		}
		result.append(")");
	}
	return result;
}

int RaceApp::getVarFilterLevel(int var_id,
		const VarsInfo::VarData& data) const {
	int num_reads = data.numReads();
	int num_writes = data.numWrites();
	if (!(num_writes >= 2 || (num_writes >= 1 && num_reads >= 1))) {
		return 0;
	}
	if (data.m_numRWRaces + data.m_numWRRaces + data.m_numWWRaces == 0) {
		return 1;
	}
	if (data.m_noParentRaces.size() == 0) {
		return 2;
	}
	RaceTags::RaceTagSet tags = m_raceTags.getVariableTags(var_id);
	if (tags != RaceTags::emptyTagSet()) {
		return 3;
	}
	bool with_undefined_init_race = m_raceTags.hasUndefinedInitilizationRace(
			var_id);
	bool with_network_race = m_raceTags.hasNetworkResponseRace(var_id, false);
	if (!with_undefined_init_race && !with_network_race) {
		return 4;
	}
	return 5;
}

std::string RaceApp::getVarTagsString(int var_id) const {
	RaceTags::RaceTagSet tags = m_raceTags.getVariableTags(var_id);
	bool with_undefined_init_race = m_raceTags.hasUndefinedInitilizationRace(
			var_id);
	bool with_network_race = m_raceTags.hasNetworkResponseRace(var_id, false);
	return StringPrintf("%s <a href=\"undef?var=%d\">%s</a> %s",
			RaceTags::tagsToString(tags).c_str(), var_id,
			with_undefined_init_race ? "initialization race1" : "",
			with_network_race ? "readyStateChange race" : "");
}

void RaceApp::showRaceLink(int race_id, std::string* response) const {
	const VarsInfo::Race_Info& race = m_vinfo.races()[race_id];
	StringAppendF(response,
			"<font color=\"red\"><b><a href=\"race?id=%d\">%s</a> race between event action "
					"<a href=\"code?focus=%d\">#%d</a> and event action "
					"<a href=\"code?focus=%d\">#%d</a></b></font>", race_id,
			race.TypeStr(), race.m_event1, race.m_event1, race.m_event2,
			race.m_event2);
}

void RaceApp::showRaceInfo(int race_id, std::string* response) const {
	const VarsInfo::Race_Info& race = m_vinfo.races()[race_id];
	StringAppendF(response, "<p>Race id #%d ; ", race_id);
	showRaceLink(race_id, response);
	if (race.m_coveredBy == -1) {
		if (race.m_multiParentRaces.empty()) {
			response->append(" - Uncovered race");
		} else {
			response->append(" (multi-covered by");
			for (size_t i = 0; i < race.m_multiParentRaces.size(); ++i) {
				if (i != 0)
					response->append(" , ");
				const VarsInfo::Race_Info& parent_race =
						m_vinfo.races()[race.m_multiParentRaces[i]];
				StringAppendF(response,
						" <a href=\"race?id=%d\">%s</a> race on <a href=\"var?id=%d\">%s</a> ",
						race.m_coveredBy, parent_race.TypeStr(),
						parent_race.m_varId,
						HTMLEscape(m_vars.getString(parent_race.m_varId)).c_str());
			}
			response->append(")");
		}
	} else {
		const VarsInfo::Race_Info& parent_race =
				m_vinfo.races()[race.m_coveredBy];
		StringAppendF(response,
				" (covered by a <a href=\"race?id=%d\">%s</a> race on <a href=\"var?id=%d\">%s</a>)",
				race.m_coveredBy, parent_race.TypeStr(), parent_race.m_varId,
				HTMLEscape(m_vars.getString(parent_race.m_varId)).c_str());
	}

	if (m_raceTags.isNetworkResponseRace(race_id)) {
		response->append(" [NET]");
	}

	response->append("</p>");
}

void RaceApp::printCommandCallTrace(int event_id, int cmd_id,
		CodeOutput* code) {
//	cout << "--printCommandCallTrace\n Event id: " << event_id << " , cmd_id: "
//			<< cmd_id << endl;
	std::vector<int> call_trace;
	m_callTraceBuilder.getCallTraceOfCommand(event_id, cmd_id, &call_trace);
	for (size_t i = 0; i < call_trace.size(); ++i) {
		int scope_cmd = call_trace[i];
		const ActionLog::EventAction& event = m_actions.event_action(event_id);
//		cout << m_scopes.getString(event.m_commands[scope_cmd].m_location)
//				<< endl;

		code->outputScopeEnter(
				m_scopes.getString(event.m_commands[scope_cmd].m_location));
	}
}

void RaceApp::Lu_printAllCommandCallTrace(int event_id, int cmd_id,
		CodeOutput* code) {
	std::vector<int> call_trace;
	m_callTraceBuilder.Lu_getCallTraceOfAllCommand(event_id, cmd_id,
			&call_trace);
	for (size_t i = 0; i < call_trace.size(); ++i) {
		int scope_cmd = call_trace[i];
		const ActionLog::EventAction& event = m_actions.event_action(event_id);
		code->outputScopeEnter(
				m_scopes.getString(event.m_commands[scope_cmd].m_location));
	}
}

void RaceApp::printCommandCallTraceFromPreviousEvents(int event_id, int cmd_id,
		CodeOutput* code, int num_previous_events) {
//	cout << "---------Eid: " << event_id << " , Cid: " << cmd_id << endl;
	int parent_event_id, parent_cmd;
	if (m_callTraceBuilder.getEventCreationCommand(event_id, &parent_event_id,
			&parent_cmd)) {
		if (num_previous_events > 0) {
			printCommandCallTraceFromPreviousEvents(parent_event_id, parent_cmd,
					code, num_previous_events - 1);
			code->outputScopeEnter("");
			code->outputScopeEnter("--triggered--");
		}
	}
	code->outputStatement("");
	code->outputStatement(
			StringPrintf("Event action <a href=\"code?focus=%d\">%d</a>",
					event_id, event_id));
	printCommandCallTrace(event_id, cmd_id, code);
}

void RaceApp::Lu_printCommandCallTraceFromPreviousEvents(int event_id,
		int cmd_id, CodeOutput* code, int num_previous_events) {
//	cout << "---------Eid: " << event_id << " , Cid: " << cmd_id << endl;
	int parent_event_id, parent_cmd;
	if (m_callTraceBuilder.getEventCreationCommand(event_id, &parent_event_id,
			&parent_cmd)) {
		if (num_previous_events > 0) {
			printCommandCallTraceFromPreviousEvents(parent_event_id, parent_cmd,
					code, num_previous_events - 1);
			code->outputScopeEnter("");
			code->outputScopeEnter("--triggered--");
		}
	}
	code->outputStatement("");
	code->outputStatement(
			StringPrintf("Event action <a href=\"code?focus=%d\">%d</a>",
					event_id, event_id));
	Lu_printAllCommandCallTrace(event_id, cmd_id, code);
}

void RaceApp::printVarAccessCallTrace(const VarsInfo::VarAccess& var_access,
		const std::string& action_str, std::string* response) {
	CodeOutput o(m_actionPrinter->function_name_printer(), response);
	printCommandCallTraceFromPreviousEvents(var_access.m_eventActionId,
			var_access.m_commandIdInEvent, &o, 0);
	o.outputStatement(action_str);

	std::string value;
	if (getAccessValue(var_access.m_eventActionId,
			var_access.m_commandIdInEvent, &value)) {
		o.outputStatement(
				StringPrintf("value <b>%s</b>", HTMLEscape(value).c_str()));
	}
}

void RaceApp::Lu_printVarAccessCallTrace(const VarsInfo::VarAccess& var_access,
		const std::string& action_str, std::string* response) {
	CodeOutput o(m_actionPrinter->function_name_printer(), response);
	Lu_printCommandCallTraceFromPreviousEvents(var_access.m_eventActionId,
			var_access.m_commandIdInEvent, &o, 0);
	o.outputStatement(action_str);

	std::string value;
	if (getAccessValue(var_access.m_eventActionId,
			var_access.m_commandIdInEvent, &value)) {
		o.outputStatement(
				StringPrintf("value <b>%s</b>", HTMLEscape(value).c_str()));
	}
}

bool RaceApp::getAccessValue(int event_action_id, int command_id,
		std::string* value) const {
	const ActionLog::EventAction& event = m_actions.event_action(
			event_action_id);
	if (command_id + 1 < static_cast<int>(event.m_commands.size())
			&& event.m_commands[command_id + 1].m_cmdType
					== ActionLog::MEMORY_VALUE) {
		*value = m_memValues.getString(
				event.m_commands[command_id + 1].m_location);
		return true;
	}
	return false;
}

void RaceApp::displayRacesIfEnabled(const URLParams& url,
		EventGraphDisplay* graph) const {
	if (url.getIntDefault("with_races", 1) != 0) {
		for (size_t i = 0; i < m_vinfo.races().size(); ++i) {
			if (m_vinfo.races()[i].m_coveredBy == -1) {
				graph->addRaceArc(i, m_vinfo.races()[i], "green");
			}
		}
	}
}

void RaceApp::showEventsSummariesIntoTable(const std::vector<int>& events,
		std::string* response) {
	int num_outputed_events = 0;
	response->append("<table>");
	for (size_t i = 0; i < events.size(); ++i) {
		if (m_actions.event_action(events[i]).m_commands.empty())
			continue;

		StringAppendF(response,
				"<tr><td>Event action <a href=\"code?focus=%d\">%d</a><br>"
						"<pre class=\"padparagraph\">", events[i], events[i]);
		m_actionPrinter->getEventActionSummary(events[i], "\n", response);
		StringAppendF(response, "</pre><br>");

		++num_outputed_events;
	}
	if (num_outputed_events == 0) {
		response->append("<tr><td>None</td></tr>");
	}
	response->append("</table>");
}

std::string RaceApp::eventActionAsStr(int event_action_id) const {
	return StringPrintf("%s (<a href=\"code?focus=%d\">%d</a>)",
			ActionLog::EventActionType_AsString(
					m_actions.event_action(event_action_id).m_type),
			event_action_id, event_action_id);
}

void RaceApp::displayNodeRelation(int node1, int node2, int cmd_in_node2,
		std::string* response) const {
	std::vector<int> considered_races;
	for (size_t i = 0; i < m_vinfo.races().size(); ++i) {
		//if (m_vinfo.races()[i].m_coveredBy == -1) {
		considered_races.push_back(i);
		//}
	}

	const EventGraphInterface* graph = m_vinfo.fast_event_graph();

	if (node1 >= node2) {
		StringAppendF(response, "<h3>Event actions %d >= %d</h3>\n", node1,
				node2);
		return;
	}

	if (graph->areOrdered(node1, node2)) {
		StringAppendF(response,
				"<h3>Event actions %d and %d are ordered.</h3>\n", node1,
				node2);
		return;
	}

	std::vector<int> targets;
	std::vector<int> parent(considered_races.size(), -2);
	std::queue<int> q;
	for (size_t i = 0; i < considered_races.size(); ++i) {
		const VarsInfo::Race_Info& race = m_vinfo.races()[considered_races[i]];
		if (graph->areOrdered(node1, race.m_event1)) {
			q.push(i);
			parent[i] = -1;  // No parent, but visited.
		}
	}
	while (!q.empty()) {
		int currId = q.front();
		q.pop();
		const VarsInfo::Race_Info& curr =
				m_vinfo.races()[considered_races[currId]];
		if (curr.m_event2 > node2)
			continue;
		if (cmd_in_node2 != -1 && curr.m_event2 == node2
				&& curr.m_cmdInEvent2 >= cmd_in_node2)
			continue;
		if (graph->areOrdered(curr.m_event2, node2)) {
			targets.push_back(currId);
		}
		for (size_t i = 0; i < considered_races.size(); ++i) {
			if (parent[i] != -2)
				continue;
			const VarsInfo::Race_Info& race =
					m_vinfo.races()[considered_races[i]];
			if (graph->areOrdered(curr.m_event2, race.m_event1)) {
				parent[i] = currId;
				q.push(i);
			}
		}
	}
	std::sort(targets.begin(), targets.end());

	if (targets.empty()) {
		StringAppendF(response,
				"<h3>Event actions %d and %d are <font color=\"red\">unordered</font>.</h3>\n",
				node1, node2);
		return;
	}

	StringAppendF(response,
			"<h3>Event actions %d and %d (until cmd %d) are likely ordered by the following covering race chains</h3>\n",
			node1, node2, cmd_in_node2);
	StringAppendF(response, "<p>%d race chains</p>\n",
			static_cast<int>(targets.size()));
	response->append("<table>");
	for (size_t i = 0; i < targets.size(); ++i) {
		int curr = targets[i];
		std::vector<int> race_chain;
		while (curr >= 0) {
			race_chain.push_back(considered_races[curr]);
			curr = parent[curr];
		}
		std::reverse(race_chain.begin(), race_chain.end());

		StringAppendF(response, "<tr class=\"ru%d\">", static_cast<int>(i % 2));
		for (size_t j = 0; j < race_chain.size(); ++j) {
			response->append("<td>");
			const VarsInfo::Race_Info& race = m_vinfo.races()[race_chain[j]];
			const char* var_name = m_vars.getString(race.m_varId);
			StringAppendF(response, "<a href=\"var?id=%d\" title=\"%s\">%s</a>",
					race.m_varId, HTMLEscape(var_name).c_str(),
					HTMLEscape(var_name).c_str());
			response->append("<br>");
			showRaceLink(race_chain[j], response);
			response->append("</td><td>");
			StringAppendF(response,
					"Event action <a href=\"code?focus=%d\">%d</a> : %d",
					race.m_event2, race.m_event2, race.m_cmdInEvent2);
			response->append("</td>");
		}
		response->append("</tr>\n");
	}
	response->append("</table>");
}

