//============================================================================
// Name        : scantool.cpp
// Author      : Ruslan Trofymenko
// Version     :
// Copyright   : 
// Description :
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>

#include <ustring.h>
#include <utime.h>
#include <App.h>

using namespace std;
using namespace sys;

typedef struct {
	string Id;
	string Data;
	unsigned int Time;
	unsigned int LastSendTime;
}SSendEntry;

unsigned int SendPackets(vector<SSendEntry>& SendList)
{
	unsigned int SleepTime = 1000;

	for (auto& Entry: SendList) {
		unsigned int Time = GetTickCount();
		unsigned int Delta = Time - Entry.LastSendTime;
		if (Delta >= Entry.Time) {
			CApp::Exec("scansend " + Entry.Id + "#" + Entry.Data);
			Entry.LastSendTime = Time;
			//cout << Entry.Id << " ";
			Delta = Entry.Time;
		}
		if (Delta < SleepTime)
			SleepTime = Delta;
	}
	//cout << endl;
	return SleepTime;
}

void SendPacketsOnce(vector<SSendEntry>& SendList)
{
	SendPackets(SendList);

	bool bFind;
	do {
		bFind = false;
		for (auto& Entry: SendList) {
			if (Entry.Time == 0) {
				SendList.erase((vector<SSendEntry>::iterator)&Entry);
				bFind = true;
				break;
			}
		}
	} while (bFind);
}

int main() {
	ifstream file("toro.txt");
	string Str;
	vector<SSendEntry> SendList;

	while (getline(file, Str)) {
		vector<string> Entries  = SplitByWaitSpace(Str);
		if (Entries.size() != 11)
			continue;

		if (Entries[0] != "frmconf")
			continue;

		if (Entries[7] != "Tx")
			continue;

		if (!IsHexValue(Entries[5]))
			continue;

		SSendEntry Entry;
		Entry.Id = Entries[5];
		ReplaceStr(Entry.Id, "0x", "");

		string TimeStr = Entries[8];
		if (TimeStr == "One") {
			Entry.Time = 0;
		} else {
			if (TimeStr.find("Cyc:") == string::npos)
				continue;

			if (TimeStr.find("msec") == string::npos)
				continue;

			ReplaceStr(TimeStr, "Cyc:", "");
			ReplaceStr(TimeStr, "msec", "");

			if (!IsIntValue(TimeStr))
				continue;

			Entry.Time = atoi(TimeStr.c_str());
		}

		if (!IsHexValue(Entries[10]))
			continue;

		Entry.Data = Entries[10];

		SendList.push_back(Entry);

		//cout << Str << endl;
		cout << Entry.Id << " " << Entry.Data << " " << Entry.Time << endl;
	}

	SendPacketsOnce(SendList);

	while (true) {
		unsigned int SleepTime = SendPackets(SendList);
		//cout << SleepTime << endl;
		if (SleepTime > 0)
			usleep(SleepTime*1000);
	}

	return 0;
}
