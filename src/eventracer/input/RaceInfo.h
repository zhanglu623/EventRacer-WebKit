#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include "ActionLog.h"
#include "../races/EventGraph.h"
#include <string>
#include <fstream>
#include <streambuf>

using namespace std;

struct Arc {

	int m_tail;
	int m_head;
	// The duration of the arc, -1 if the duration is unknown.
	int m_duration;
};

struct TokenInfo {
	int eid;
	int lineNumber;
	string source;
	int lineIndex; //which token of the line
	string tokenContent;

	string print();
};
struct TimerInfo {

	TimerInfo();

	int eid;
	string timerType;
	string timerPtr;

	int nestedInEvent;
	int containEvent;

	int childIndex;
	map<string,int> setTimersPtr;

	string print();
};

class RaceHandler {
public:

	static RaceHandler* Instance();

	void readZlLog(char* file);
	void readNetwork(char* file);
	void readEvent(char* file);
	string readRaceTemplete(char* file);

	string trimString(string str);
	void readIds(char* file);
	map<string,string> id_ptr;
	map<string,string> ptr_id;
	map<string,string> id_html;

	vector<string> split_string(string input, string split_by);
	void str_replace(string &s, const string &search, const string &replace);

	map<int ,pair<string, pair<string, string> > > op_to_raceInfo;

	string getWholeElement(int eid);
	void readToken(char* file);
	string originalHTML;

	bool isToken(int eid);
	string getTokenInfo(int eid);
	bool isTimer(int eid);

	void readTimer(char* file);
	void findThreadInfo(int eid);

	int getLastTimerEventId(int eid);

	void recordReplayTrace(int early, int late);
	void recordReplayTraceNew(int early, int late);

	void findRaceThreadInfo(const ActionLog& actions,
			const SimpleDirectedGraph& EventGraph,
			const SimpleDirectedGraph& EventTimerGraph);

	void removeNonTimerSuccessors();
	int findParsingPredecessors(int eid);

	int startFindParsingPredecessors(int eid);
	void finishFindParsingPredecessors(int eid);

	string printEventBackTrackTrace(int eid);
	void logToFile(char* file, string content);
	void logToRace(char* file, string content);

	int findRootOfNestedEvent(int eid);

public:
	map<int, int> EventActionSet;
	map<int, map<int, pair<string, int> > > succSet; //map<pred, map<succ, map<type, index> > >
	map<int, map<string, map<int, int> > > succSetTypeIndex; //map<pred, map<succ, map<type, index> > >
	map<int, map<int, int> > predSet; //

	map<int, int> predSetOfParsingEvent;

	map<int, set<int> > ThreadActionSet;
	map<int, int> ActionThreadSet;
	set<pair<int, int> > races;
//	map<int, pair<string, int> > timerEventInfo; //map<eid, pair<timerId, count> >
//	map<int, vector<int> > nestedEvents;
	map<int, string> networkEvents;
	map<int, TokenInfo> Tokens;
	map<int, TokenInfo> reverseTokens;
	map<int, TimerInfo> timerEvents;
	map<string, int> timerSetByEvent;
	map<int, vector<int> > EventBackTrackTrace; //for an normal event, is will be : e1->pred->pred...->parsing, for parsing event, it will be e2->previous parsing event
	bool WhetherGerneateRaceLogs;
private:

	RaceHandler();

};

