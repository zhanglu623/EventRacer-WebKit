#include <iostream>
#include <fstream>
#include "RaceInfo.h"
#include <stdlib.h>     /* atoi */
#include <assert.h>
#include <sstream>
#include <climits>
#include <algorithm>
using namespace std;
static RaceHandler* m_pInstance = NULL;

static bool DEBUG = false;

RaceHandler::RaceHandler() {
	cout << "RaceInfo\n";
	WhetherGerneateRaceLogs = true;
}

RaceHandler* RaceHandler::Instance() {
	if (!m_pInstance)   // Only allow one instance of class to be generated.
		m_pInstance = new RaceHandler;
	return m_pInstance;
}

void RaceHandler::recordReplayTrace(int early, int late) { //in the replay run, e1 should fire before e2
	static bool reset = false;
	if (!reset) {
//		system("cd ../webkit/replay; rm *.log");
//		system("cd ../webkit/replay/done; rm *.log");
		system("rm -rf ../webkit/replay; mkdir ../webkit/replay");
		system("rm -rf ../webkit/replay/done; mkdir ../webkit/replay/done");
		reset = true;
	}
	static int count = 1;

	char file[1024];

	sprintf(file, "../webkit/replay/replay%d.log", count);

	ofstream myfile;
	myfile.open(file, ios::out);
	myfile << timerEvents[early].timerType << "\n";
	myfile << timerEvents[early].childIndex << "\n";
	myfile << timerEvents[late].timerType << "\n";
	myfile << timerEvents[late].childIndex << "\n";
	myfile.close();

//	sprintf(file, "../webkit/replay/replay%d.log", count*10+1);
//
//	myfile.open(file, ios::out);
//
//	myfile << timerEventInfo[late].first << "\n";
//	myfile << timerEventInfo[late].second << "\n";
//	myfile << timerEventInfo[early].first << "\n";
//	myfile << timerEventInfo[early].second << "\n";

	myfile.close();

	count++;
}

void RaceHandler::recordReplayTraceNew(int early, int late) { //in the replay run, e1 should fire before e2
	if (RaceHandler::Instance()->WhetherGerneateRaceLogs) {
		static bool reset = false;
		if (!reset) {
//		system("cd ../webkit/replay; rm *.log");
//		system("cd ../webkit/replay/done; rm *.log");
			system("rm -rf ../webkit/replay/replay*; mkdir ../webkit/replay");
			system("rm -rf ../webkit/replay/done; mkdir ../webkit/replay/done");
			reset = true;
		}
	}
	static int count = 1;

	char file[1024];

	sprintf(file, "../webkit/replay/replay%d.log", count);

	cout << "File: " << (string) file << "\n";

	////////////////////////////////////////////////////////////
	if (DEBUG) {
		cout << "\n*************For :" << early << "\n";
	}
	int e1Parser = RaceHandler::Instance()->startFindParsingPredecessors(early);
	if (DEBUG) {
		cout << "\n*************For :" << late << "\n";
	}
	int e2Parser = RaceHandler::Instance()->startFindParsingPredecessors(late);

	if (e1Parser == -1 || e2Parser == -1) {
		cout << "Error, not all events find a parsing predecessor!\n";
		return; //one of the event doesn't find the parsing predecessor
	}

//	cout << "Psrsing tokens: \n";
//	cout << "For E1: " << early << ", the parsing event is :" << e1Parser
//			<< "\n";
//	cout << RaceHandler::Instance()->Tokens[e1Parser].print();
//	cout << "For E1: " << late << ", the parsing event is :" << e2Parser
//			<< "\n";
//	cout << RaceHandler::Instance()->Tokens[e2Parser].print();

//	cout<<"----\n";

	///////////////////////////////////////////////////////////

	stringstream ss;
	//

	ss << "Event: " << early << endl;
	ss << "Begin\n";
	ss << printEventBackTrackTrace(early);
	ss << "End\n";
	ss << "Size: " << EventBackTrackTrace[early].size() << endl;
	ss << "\n";

	ss << "Event: " << late << endl;
	ss << "Begin\n";
	ss << printEventBackTrackTrace(late);
	ss << "End\n";
	ss << "Size: " << EventBackTrackTrace[late].size() << endl;

	if (RaceHandler::Instance()->WhetherGerneateRaceLogs) {
		RaceHandler::Instance()->logToFile("race", ss.str());
		ofstream myfile;
		myfile.open(file, ios::out);
		myfile << ss.str();
		myfile.close();
	} else {
		RaceHandler::Instance()->logToFile("race_no", ss.str());
	}
	count++;
}

//int RecursivelyFindTimerPredecessor(int eid){
//	for()
//}

//int RaceHandler::getLastTimerEventId(int eid) {
//	int ret = -1;
//	printf("---------------------------------------\n");
//	if (timerEventInfo.find(eid) != timerEventInfo.end()) {
//		printf("E:----Eid=%d\n   ----Index=%d\n   ----Type=%s\n", eid,
//				timerEventInfo[eid].second, timerEventInfo[eid].first.c_str());
//		ret = eid;
//	} else {
//		printf(
//				"   ----Event: %d is not a timer event, need to find the predecessor\n",
//				eid);
//		for (vector<int>::iterator it = nestedEvents[eid].begin();
//				it != nestedEvents[eid].end(); ++it) {
//			cout << "Predecessor: " << *it << "\n";
//			if (timerEventInfo.find((*it)) != timerEventInfo.end()) {
////				cout<<"Predecessor: " <<*it;
//				printf(
//						"----1: Predecessor of E:----Eid=%d\n   ----Index=%d\n   ----Type=%s\n",
//						(*it), timerEventInfo[(*it)].second,
//						timerEventInfo[(*it)].first.c_str());
//				ret = (*it);
//				break;
//			}
//		}
//
//		if (ret == -1) {
//			printf("Couldn't find nested predecessor, try predecessors\n");
//			cout << "Regular pred size: " << predSet[eid].size() << "\n";
//			int tempPred = eid;
//			while (predSet[tempPred].size() == 1) {
//				tempPred = (*predSet[tempPred].begin()).first;
//				if (timerEventInfo.find(tempPred) != timerEventInfo.end()) {
//
//					printf(
//							"----2: Predecessor of E:----Eid=%d\n   ----Index=%d\n   ----Type=%s\n",
//							(tempPred), timerEventInfo[tempPred].second,
//							timerEventInfo[tempPred].first.c_str());
//
//					ret = tempPred;
//					break;
//
//				}
//
//			}
//
//		}
//
//	}
////	assert(eid != -1);
//
//	if (ret == -1) {
//		printf("Wrong!\n");
//	}
//
//	return ret;
//}

void RaceHandler::readZlLog(char* file) {
//	string line;
//	int eid;
//	string timerType;
//	int index;
//	ifstream myfile(file);
//	if (myfile.is_open()) {
//		while (getline(myfile, line)) {
//
//			if (line.compare("Timer Log") == 0) {
//				getline(myfile, line);
//				eid = atoi(line.c_str());
//
//				getline(myfile, line);
//				timerType = line;
//
//				getline(myfile, line);
//				index = atoi(line.c_str());
//
//				getline(myfile, line);
////				cout << "Eid:  " << eid << "  Index: " << index << "  Type:  "
////						<< timerType << "\n";
//
//				timerEventInfo[eid] = make_pair(timerType, index);
//			} else if (line.compare("Nested Events") == 0) {
//				getline(myfile, line);
//				eid = atoi(line.c_str());
////				cout << "Event: " << eid << " is nested with:  ";
//				while (getline(myfile, line)) {
//					if (line.compare("------------------------------") != 0) {
//						int temp = atoi(line.c_str());
//						nestedEvents[eid].push_back(temp);
//						cout << temp << " , ";
//					} else {
//						break;
//					}
//				}
////				cout << "\n";
//
//			}
//
//		}
//		myfile.close();
//	}

}

void RaceHandler::readNetwork(char* file) {
	string line;
	int eid;
	string url;
	ifstream myfile(file);
	if (myfile.is_open()) {
		while (getline(myfile, line)) {

			eid = atoi(line.c_str());

			getline(myfile, line);
			url = line;
			networkEvents[eid] = url;

			getline(myfile, line);

//			cout << "Eid: " << eid << "\n";
//			cout << "Url: " << url << "\n";

		}
		myfile.close();
	}

}

void RaceHandler::str_replace(string &s, const string &search,
		const string &replace) {
	for (size_t pos = 0;; pos += replace.length()) {
		pos = s.find(search, pos);
		if (pos == string::npos)
			break;

		s.erase(pos, search.length());
		s.insert(pos, replace);
	}
}

void RaceHandler::readEvent(char* file) {

//	map<int, TimerInfo> timerEvents;
//	map<string, int> timerSetByEvent;
	string line = "";
	int eid = 0;
	string timerPtr;
	string timerType = "";
	ifstream myfile(file);
	if (myfile.is_open()) {
		while (getline(myfile, line)) {

//			cout << line << endl;

			if (line.compare("Timer set :") == 0) {

			}

			else if (line.substr(0, 5).compare("Start") == 0) {
				//Start: 6

				TimerInfo newItem;

				int eid = atoi(line.substr(7, line.length() - 7).c_str());

				newItem.eid = eid;
				newItem.setTimersPtr.clear();

//				cout << newItem.eid << endl;

				while (getline(myfile, line)) {
					if (line.compare("Timer set :") == 0) {
						getline(myfile, line);
						timerPtr = line.substr(6, line.length() - 6);

//						cout << timerPtr << endl;

						getline(myfile, line);
						timerType = line;

//						cout << newItem.setTimersPtr.size() << endl;

						if (newItem.setTimersPtr.find(timerPtr)
								== newItem.setTimersPtr.end()) {
//							cout << newItem.setTimersPtr.size() << endl;
							newItem.setTimersPtr[timerPtr] =
									newItem.setTimersPtr.size();
//							cout << newItem.setTimersPtr[timerPtr] << endl;
						}
//						cout << newItem.setTimersPtr.size() << endl;
						timerSetByEvent[timerPtr] = newItem.eid;
						continue;
					}

					if (line.compare("--->Timer fire:") == 0) {
						getline(myfile, line);
						timerPtr = line.substr(6, line.length() - 6);

//						cout << timerPtr << endl;

						getline(myfile, line);
						timerType = line;

//						cout << timerType << endl;

						getline(myfile, line);
						eid = atoi(line.substr(6, line.length() - 6).c_str());

//						cout << eid << endl;

						newItem.timerType = timerType;
						newItem.timerPtr = timerPtr;

						assert(eid = newItem.eid);

						continue;
					}

					if (line.substr(0, 5).compare("Start") == 0) {
						eid = atoi(line.substr(6, line.length() - 6).c_str()); //nested

						newItem.containEvent = eid;

						TimerInfo newItem2;
						newItem2.eid = eid;
						newItem2.nestedInEvent = newItem.eid;

						timerEvents[newItem2.eid] = newItem2;
					}

					if (line.compare(
							"--------------------------------------------------")
							== 0) {

						break;
					}
				}

				timerEvents[newItem.eid] = newItem;

			} else {
				continue;
			}

		}
		myfile.close();
	}

//	for (map<int, TimerInfo>::iterator it = timerEvents.begin();
//			it != timerEvents.end(); ++it) {
//
//		cout << (*it).second.print() << endl;
//
//	}
}

string RaceHandler::readRaceTemplete(char* file) {

	static string templete = "";
	if (!templete.empty())
		return templete;

//	string retVal = "";

	std::ifstream t(file);
	std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
//	cout<<str<<endl;

//	if (str.empty())
//		return "";

	templete = str;
	return templete;

}

string RaceHandler::trimString(string str) {
	if (str.empty())
		return "";
	str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
	return str;
}

void RaceHandler::readIds(char* file) {

	std::ifstream t(file);
	std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
//	cout<<str<<endl;

	if (str.empty())
		return;

	str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());

	vector<string> elements = split_string(str, "|||||");

	for (vector<string>::iterator it = elements.begin(); it != elements.end();
			++it) {
//		cout<<"---"<<(*it)<<endl;

		if ((*it).empty())
			continue;

		vector<string> infos = split_string((*it), "||||");

		string ptr = infos[0];
		string id = infos[1];
		string html = infos[2];

//		cout << "\n---" << ptr << endl;
//		cout << "---" << id << endl;
//		cout << "---" << html << endl;

		id_ptr[id] = ptr;
		ptr_id[ptr] = id;
		id_html[id] = html;

	}

//	for(map<string,string>::iterator it=ptr_id.begin();it!=ptr_id.end();++it){
//		cout<<(*it).first<<" : "<<(*it).second<<endl;
//	}

}

int RaceHandler::findRootOfNestedEvent(int eid) {
	int retVal = eid;
	if (timerEvents.find(eid) != timerEvents.end()) {
		if (timerEvents[eid].nestedInEvent != -1) {
			retVal = timerEvents[eid].nestedInEvent;
		}
	}
	return retVal;
}

bool RaceHandler::isToken(int eid) {
	if (Tokens.find(eid) == Tokens.end())
		return false;
	else
		return true;
}

string RaceHandler::getTokenInfo(int eid) {
	stringstream ss;
	if (isToken(eid)) {
		TokenInfo& token = Tokens[eid];
		ss << "-------------------------------------------------\n";
		ss << "Eid        : " << token.eid << "\n";
		ss << "Line Number: " << token.lineNumber << "\n";
		ss << "Source     : " << token.source << "\n";
		ss << "Index      : " << token.lineIndex << "\n";
		ss << "\n";
	}
	return ss.str();

}

bool RaceHandler::isTimer(int eid) {

	if (timerEvents.find(eid) != timerEvents.end()
			&& timerEvents[eid].timerType.compare("NotTimer") != 0)
		return true;
	else
		return false;

}

string RaceHandler::getWholeElement(int eid) {
	eid = INT_MAX - eid;
	string retVal = "";
	map<int, TokenInfo>::iterator it = reverseTokens.find(eid);
	string src = (*it).second.source;

	cout << "Original HTML : " << originalHTML << endl;
	cout << "Current  HTML : " << src << endl;

	if (src.compare(originalHTML) != 0) {
		retVal = "Src:::" + src;
		return retVal;
	}

	for (; it != Tokens.end(); ++it) {
//		cout<<"Try id: "<<(*it).second.eid<<endl;
		if ((*it).second.source.compare(src) != 0) {
			continue;
		}
		string tokenHTML = (*it).second.tokenContent;
//		if (tokenHTML.empty())
//			break;

//		tokenHTML = trimString(tokenHTML);

//		cout << (*it).second.eid << ",-- :" << (*it).second.tokenContent << endl;

//		break;

		int hasIdPos = tokenHTML.find("id=");
		if (hasIdPos != std::string::npos) {

			string temp = tokenHTML.substr(hasIdPos + 4);
//			cout << (*it).second.eid << ",-- :" << temp << endl;

			temp = temp.substr(0, temp.find("\""));
//			cout << (*it).second.eid << ",-- :" << temp << endl;

			return temp;

			break;
		}
	}

//	for (map<int, TokenInfo>::iterator it = reverseTokens.begin();
//			it != reverseTokens.end(); ++it) {
//		cout << (*it).second.eid << ",-- " << (*it).second.tokenContent << endl;
//	}

//	map<int, TokenInfo>::iterator it = reverseTokens.find(eid);
//	if(it!=reverseTokens.end()){
//		cout << (*it).second.eid << ",-- " << (*it).second.tokenContent << endl;
//	}
//

	return retVal;
}

void RaceHandler::readToken(char* file) {
	originalHTML = "";
	string line;

	int currEid;
	int lineNumber;
	string htmlSource;
	string content;

	static int lastTimer = -1;

	TokenInfo token;

	ifstream myfile(file);
	if (myfile.is_open()) {
		while (getline(myfile, line)) {

//			getline(myfile, line);
			line = line.substr(12);
			currEid = atoi(line.c_str());

			getline(myfile, line);
			line = line.substr(12);
			lineNumber = atoi(line.c_str());

			getline(myfile, line);
			line = line.substr(12);
			htmlSource = line.c_str();

			if (originalHTML.empty()) {
				if (!htmlSource.empty()) {
					originalHTML = htmlSource;
				}
			}

			getline(myfile, line);
			line = line.substr(12);
			content = line.c_str();

			while (true) {
				getline(myfile, line);
				if (line.find("--Token src end") == std::string::npos) {
					content = content.append(line);
				} else {
					break;
				}
			}

			token.eid = currEid;
			token.lineNumber = lineNumber;
			token.source = htmlSource;
			token.tokenContent = content;

			if (Tokens.find(token.eid) != Tokens.end()) {
				Tokens[token.eid].tokenContent.append(token.tokenContent);

			} else {
				Tokens[token.eid] = token;
			}
			predSetOfParsingEvent[currEid] = lastTimer;

//			cout << "-------------------------------------------------\n";
//			cout << "Eid        : " << token.eid << "\n";
//			cout << "Line Number: " << token.lineNumber << "\n";
//			cout << "Source     : " << token.source << "\n";
//			cout << "Content    : " << token.tokenContent << "\n";
//			cout << "\n";
			getline(myfile, line);

			lastTimer = currEid;
		}
		myfile.close();
	}

	for (map<int, TokenInfo>::reverse_iterator it = Tokens.rbegin();
			it != Tokens.rend(); ++it) {
		reverseTokens[INT_MAX - (*it).first] = (*it).second;
	}

//	for (map<int, TokenInfo>::iterator it = reverseTokens.begin();
//			it != reverseTokens.end(); ++it) {
//		cout << (*it).second.eid << ",-- " << (*it).second.tokenContent << endl;
//	}

}

vector<string> RaceHandler::split_string(string input, string split_by) {

//	cout << "input: " << input << endl;

	vector<string> str_list;
	if (input.size() < 1)
		return str_list;
	int comma_n = 0;
	while (true) {
		std::string tmp_s = "";
		comma_n = input.find(split_by);
		if (-1 == comma_n) {
			tmp_s = input.substr(0, input.length());
			str_list.push_back(tmp_s);
			break;
		}
		tmp_s = input.substr(0, comma_n);
		input.erase(0, comma_n + split_by.size());
		str_list.push_back(tmp_s);
//		cout<<"tmp_s: "<<tmp_s<<endl;
	}
	return str_list;
}

void RaceHandler::readTimer(char* file) {
//
//	string line;
//
//	int currEid;
//
//	string timerType;
//
//	TokenInfo token;
//
//	ifstream myfile(file);
//	if (myfile.is_open()) {
//		while (getline(myfile, line)) {
//
//			TimerInfo newTimer;
//
////			getline(myfile, line);
//			currEid = atoi(line.c_str());
//			newTimer.eid = currEid;
//
//			getline(myfile, line);
//			timerType = line.c_str();
//			newTimer.type = timerType;
//
//			getline(myfile, line);
//			newTimer.timerId = atoi(line.c_str());
//
//			getline(myfile, line);
//			if (line.compare("[") == 0) {
//				while (getline(myfile, line)) {
//					if (line.compare("]") == 0) {
//						break;
//					} else {
//						newTimer.forkTimers.push_back(atoi(line.c_str()));
//					}
//				}
//			}
//
//			timerEvents[currEid] = newTimer;
//
////			cout << "-------------------------------------------------\n";
////			cout << "Eid        : " << currEid << "\n";
////			cout << "Timer Type : " << timerType << "\n";
////
////			cout << "\n";
//			getline(myfile, line);
//
//		}
//		myfile.close();
//	}

}

//void RaceHandler::findRaceThreadInfo(const ActionLog& actions,
//		const SimpleDirectedGraph& EventGraph,
//		const SimpleDirectedGraph& EventTimerGraph) {
////	set<pair<int, int> > races;
//
//	set<pair<int, int> >::iterator raceIterator;
//	int event1;
//	int event2;
//
//	for (raceIterator = races.begin(); raceIterator != races.end();
//			++raceIterator) {
//		event1 = (*raceIterator).first;
//		event2 = (*raceIterator).second;
//
//	}
//
//}

//void RaceHandler::removeNonTimerSuccessors() {
//	map<int, set<int> >::iterator itSucc;
//
//	for (itSucc = succSet.begin(); itSucc != succSet.end(); ++itSucc) {
//		int eid = (*itSucc).first;
//
//		for (set<int>::iterator it = (*itSucc).second.begin();
//				it != (*itSucc).second.end(); ++it) {
//			int Successor = (*it); //the sucessor
//
//			if (timerEvents.find(Successor) == timerEvents.end()) { // the non timer succesor
//
//				(*itSucc).second.insert(succSet[Successor].begin(),
//						succSet[Successor].end()); // merge the non-timer successor's sucessors into the eid's sucessor
//
//				(*itSucc).second.erase(eid); //remove the non timer successor from eid
//
//			}
//
//		}
//	}
//
//}

static map<int, bool> traversed;
string spaces = "";
static int currentBacktracingEvent = -1;
int RaceHandler::startFindParsingPredecessors(int eid) {
	currentBacktracingEvent = eid;
	EventBackTrackTrace[currentBacktracingEvent].clear();
	int ret = findParsingPredecessors(eid);

	finishFindParsingPredecessors(eid);
	return ret;
}
void RaceHandler::finishFindParsingPredecessors(int eid) {
	currentBacktracingEvent = -1;
}

int RaceHandler::findParsingPredecessors(int eid) {
//	map<int, set<int> > succSet;//
//	map<int, set<int> > predSet;//
//	map<int, set<int> >::iterator itSucc;

	map<int, int>::iterator itPred;
	int spaceLength = spaces.length();

	if (spaceLength == 0) {
		if (isToken(eid)) {
			if (DEBUG) {
				cout << spaces << "----Self: " << eid << " is Token!!! \n";
			}
			spaces = "";

			//This is a parsing event, we need to add the previous parsing event into it's backtrace
			EventBackTrackTrace[currentBacktracingEvent].push_back(
					predSetOfParsingEvent[currentBacktracingEvent]);

			EventBackTrackTrace[currentBacktracingEvent].push_back(
					currentBacktracingEvent);

			return eid;
		}
		if (DEBUG) {
			if (isTimer(eid)) {
				cout << spaces << "----Event: " << eid << " is Timer!!! \n";
			} else {
				cout << spaces << "----Event: " << eid << " is Not Timer!!! \n";
			}
		}
	}

	spaces = spaces.append("  ");
	int pred;
	if (predSet[eid].size() > 1) {
		pred = -1;
		if (isTimer(eid)) {

			for (itPred = predSet[eid].begin(); itPred != predSet[eid].end();
					++itPred) {

				int tempPred = (*itPred).first;

//				cout<<timerEvents[eid].timerPtr<<"\n";
//				for (map<string, int>::iterator it =
//						timerEvents[tempPred].setTimersPtr.begin();
//						it != timerEvents[tempPred].setTimersPtr.end(); ++it) {
//
//					cout<<(*it).first<<" : "<<(*it).second<<"\n";
//
//				}

				if (timerEvents[tempPred].setTimersPtr.find(
						timerEvents[eid].timerPtr)
						!= timerEvents[tempPred].setTimersPtr.end()) {

					pred = tempPred;

					break;

				}

			}

		}
		if (pred == -1) {
			return -1; //multiple preds
		}
//
//		std::stringstream ss;
//		ss << "Pred size of event :" << eid << " , is " << predSet[eid].size()
//				<< "\n";
//
//		RaceHandler::Instance()->logToFile("race", ss.str());

	} else {
		for (itPred = predSet[eid].begin(); itPred != predSet[eid].end();
				++itPred) {

			pred = (*itPred).first;
		}
	}

//		cout << spaces << "Pred: " << pred << "\n";

	if (DEBUG) {
		if (isTimer(pred)) {
			cout << spaces << "Pred: " << pred << " [is  a  Timer] .";
		} else {
			cout << spaces << "Pred: " << pred << " [is Not Timer] .";
		}
	}
	if (isToken(pred)) {
		if (DEBUG) {
			cout << " [is  a  Token!!!] Stop\n";
		}
		spaces = "";
		EventBackTrackTrace[currentBacktracingEvent].push_back(pred);
		EventBackTrackTrace[currentBacktracingEvent].push_back(eid);
		return pred;
	}

	if (!isToken(pred)) {
		if (DEBUG) {
			cout << " [is Not Token!!!] continue traversing \n";
		}

		int ret = findParsingPredecessors(pred);

		if (ret != -1) {
			spaces = "";
			EventBackTrackTrace[currentBacktracingEvent].push_back(eid);
			return ret;
		}
	}

	spaces = spaces.substr(0, spaceLength);

	return -1;
}

string TokenInfo::print() {
	stringstream ss;
	ss << this->lineNumber << "\n";
	ss << this->lineIndex << "\n";

	return ss.str();
}

TimerInfo::TimerInfo() {
	eid = -1;
	timerType = "NotTimer";
	timerPtr = "-1";

	nestedInEvent = -1;
	containEvent = -1;

	childIndex = -1;
}

string TimerInfo::print() {

//	struct TimerInfo {
//		int eid;
//		string timerType;
//		string timerPtr;
//
//		int nestedInEvent;
//		int containEvent;
//
//		int childIndex;
//		vector<string> setTimersPtr;
//
//		string print();
//	};

	stringstream ss;
	ss << "Eid          : " << this->eid << "\n";
	ss << "timerType    : " << this->timerType << "\n";
	ss << "timerPtr     : " << this->timerPtr << "\n";
	ss << "nestedInEvent: " << this->nestedInEvent << "\n";
	ss << "containEvent : " << this->containEvent << "\n";
	ss << "setTimersPtr : " << this->setTimersPtr.size() << "\n";
	if (setTimersPtr.size() > 0) {

		for (map<string, int>::iterator it = setTimersPtr.begin();
				it != setTimersPtr.end(); ++it) {
			ss << "set----->>: " << (*it).first << ",  order: " << (*it).second
					<< "\n";
		}
	}
	return ss.str();
}

string RaceHandler::printEventBackTrackTrace(int eid) {
	stringstream ss;
//	map<int, vector<int> > EventBackTrackTrace;

//	string pringSpace = "  --";
//	spaces = spaces.append("  ");

	int lastEvent = -1;
	cout << "Event :" << eid << "\n";
	for (vector<int>::iterator it = EventBackTrackTrace[eid].begin();
			it != EventBackTrackTrace[eid].end(); ++it) {
		if (lastEvent != -1) {
//			cout<<"->\n";

			if (timerEvents[*it].timerPtr.compare("-1") != 0) { //is a timer event

				ss << "Child :controllable\n";
				ss << succSet[lastEvent][(*it)].second << "\n";
				ss << succSet[lastEvent][(*it)].first << "\n";

				if (timerEvents[lastEvent].setTimersPtr.find(
						timerEvents[*it].timerPtr)
						!= timerEvents[lastEvent].setTimersPtr.end()) {

					ss
							<< timerEvents[lastEvent].setTimersPtr[timerEvents[*it].timerPtr]
							<< "\n";

				}
				assert(
						timerEvents[*it].timerType.compare(
								succSet[lastEvent][(*it)].first) == 0);

			} else {
				ss << "Child :non-controllable\n";
				ss << succSet[lastEvent][(*it)].second << "\n";
				ss << succSet[lastEvent][(*it)].first << "\n";
			}

//			cout<<"\n";
//			cout << succSet[lastEvent][(*it)].second
//					<< " th child,    type is: "
//					<< succSet[lastEvent][(*it)].first << "\n";
		} else {
			ss << "Token:\n";
			ss << Tokens[(*it)].print();
		}

//		cout << pringSpace << (*it) << "\n";

//		pringSpace = pringSpace.append("--");
		lastEvent = (*it);
	}

//	ss<<"\n";
//	cout << ss.str();
	return ss.str();
}

void RaceHandler::logToFile(char* file, string content) {

//	replayTrace::Instance();

	char Command[1024];

	static map<string, bool> done;
//	static bool done = false;
	if (done.find((string) file) == done.end()) {
		system("mkdir DebugInfo");
		sprintf(Command, "mv DebugInfo/%s2 DebugInfo/%s3", file, file);
		system(Command);
		sprintf(Command, "mv DebugInfo/%s DebugInfo/%s2", file, file);
		system(Command);
		done[(string) file] = true;
	}
	sprintf(Command, "DebugInfo/%s", file);
	ofstream myfile(Command, ios::app);
	if (myfile.is_open()) {
//				myfile << "countPumpTokenizer: " << countPumpTokenizer
//						<< "   , countAddArc: " << countAddArc << "\n";
//				myfile << countOriginalToken << " : " << LineNumber << " : "
//						<< sourceForToken(m_token).toStdString() << "\n";

		myfile << content << "\n";

		myfile.close();
	} else
		cout << "Unable to open file";
}

void RaceHandler::logToRace(char* file, string content) {

//	replayTrace::Instance();

	char Command[1024];

	static map<string, bool> done;
//	static bool done = false;
	if (done.find((string) file) == done.end()) {
		system("mkdir races");
		sprintf(Command, "mv races/%s2 races/%s3", file, file);
		system(Command);
		sprintf(Command, "mv races/%s races/%s2", file, file);
		system(Command);
		done[(string) file] = true;
	}
	sprintf(Command, "races/%s", file);
	ofstream myfile(Command, ios::app);
	if (myfile.is_open()) {
//				myfile << "countPumpTokenizer: " << countPumpTokenizer
//						<< "   , countAddArc: " << countAddArc << "\n";
//				myfile << countOriginalToken << " : " << LineNumber << " : "
//						<< sourceForToken(m_token).toStdString() << "\n";

		myfile << content << "\n";

		myfile.close();
	} else
		cout << "Unable to open file";
}
