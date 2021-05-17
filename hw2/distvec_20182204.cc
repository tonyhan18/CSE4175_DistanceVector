/****************************************/
/* title : distvec_20182204.cc          */
/* developer : Han Chan Hee(20182204)   */
/* date : 2021.05.15                    */
/****************************************/

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <algorithm>
#define LIMIT " \t\r\n\f\v"
#define MAX_READ 1001
using namespace std;

typedef int routerip_t;
typedef int cost_t;

typedef struct RoutingTableEntry {
	routerip_t    nexthop;
	cost_t        cost;

	RoutingTableEntry(routerip_t nexthop, cost_t cost) : nexthop(nexthop), cost(cost) {}
}RTE;

typedef struct _packet {
	routerip_t  src;
	routerip_t  dest;
	vector<int> hop;
	char        msg[MAX_READ];
	cost_t      cost;

	_packet() :src(0), dest(0), cost(0) {}
	_packet(routerip_t src, routerip_t dest, char* msg)
		: src(src), dest(dest), cost(0)
	{
		strncpy(this->msg, msg, MAX_READ);
		hop.push_back(src);
	}
}PACKET;

//dest : key, nexthop,cost : value
map<routerip_t, RTE>* g_table[101];
vector<int> g_map[101];
int     g_size;
vector<vector<int> > g_origin(101, vector<int>(101, -999));
//int		g_origin[101][101];
//map<routerip_t, RTE>* dvt;


//테스트용 테이블 출력
void    printRoutingTable_win(void)
{
	for (int i = 0; i < g_size; ++i)
	{
		map<routerip_t, RTE>::iterator it = g_table[i]->begin();
		for (it = g_table[i]->begin(); it != g_table[i]->end(); ++it)
		{
			cout << it->first << ' ' << it->second.nexthop << ' ' << it->second.cost << '\n';
		}
		cout << '\n';
	}
}

//파일에 테이블 출력
void    printRoutingTable(ofstream& output)
{
	for (int i = 0; i < g_size; ++i)
	{
		map<routerip_t, RTE>::iterator it;
		for (it = g_table[i]->begin(); it != g_table[i]->end(); ++it)
		{
			if (it->second.cost == -999) continue;
			output << it->first << ' ' << it->second.nexthop << ' ' << it->second.cost << '\n';
		}
		output << '\n';
	}
}

void	printMap()
{
	for (int i = 0; i < g_size; ++i)
	{
		vector<int>::iterator it = g_map[i].begin();
		cout << i << " = ";
		for (; it < g_map[i].end(); ++it)
		{
			cout << *it << ' ';
		}
		cout << '\n';
	}
}

//노드간 테이블 교환
void    xchgTable(void)
{
	map<routerip_t, RTE>::iterator s_it;
	map<routerip_t, RTE>::iterator d_it;

	//전체 트리를 탐색
	for (int src = 0; src < g_size; ++src)
	{
		for (vector<int>::iterator dest = g_map[src].begin(); dest < g_map[src].end(); ++dest)
		{
			//각자 서로의 위치로 갈때의 cost를 미리 저장 -> 반드시 존재
			int s_to_d_cost = g_origin[src][*dest];
			int d_to_s_cost = g_origin[*dest][src];

			//src를 dest 테이블을 기준으로 바꾸기
			for (d_it = g_table[*dest]->begin(), s_it = g_table[src]->begin(); d_it != g_table[*dest]->end(); d_it++, s_it++)
			{
				//자기자신에 대한 내용인 경우
				if (d_it->first == *dest) continue;
				if (s_it->first == src) continue;

				//dest도 목적지를 가기 위해 어떻게 가야하는지 모르는 경우
				if (d_it->second.nexthop == -999 && d_it->second.cost == -999) {}
				// src 테이블 바꾸기
				// cost가 없었거나 해당 노드를 1차로 밟고 가는 경우만
				else if ((s_it->second.nexthop == -999 && s_it->second.cost == -999) || (s_it->second.cost >= s_to_d_cost + d_it->second.cost))
				{
					//cout<<"src : "<<src<<"="<<*dest<<' '<< s_to_d_cost + d_it->second.cost<<'\n';
					s_it->second.nexthop = *dest;
					s_it->second.cost = s_to_d_cost + d_it->second.cost;
				}

				//src도 목적지를 가기 위해 어떻게 가야하는지 모르는 경우
				if (s_it->second.nexthop == -999 && s_it->second.cost == -999) {}
				// dest 테이블 바꾸기
				else if ((d_it->second.nexthop == -999 && d_it->second.cost==-999) || (d_it->second.cost >= d_to_s_cost + s_it->second.cost))
				{
					//cout << "dest : " << *dest<<"=" << src << ' ' << d_to_s_cost + s_it->second.cost << '\n';
					d_it->second.nexthop = src;
					d_it->second.cost = d_to_s_cost + s_it->second.cost;
				}

			}
		}
	}
}

//topology 읽기, graph 그리기
bool    initRouter(ifstream& topology, ofstream& output)
{
	routerip_t src_Router, dest_Router;
	cost_t cost;

	if (!topology) return (false);
	topology >> g_size;
	//자기자신 초기화
	for (int i = 0; i < g_size; ++i)
	{
		g_table[i] = new map<routerip_t, RTE>;
		g_table[i]->insert(map<routerip_t, RTE>::value_type(i, RTE(i, 0)));
	}
	while (!topology.eof())
	{
		topology >> src_Router >> dest_Router >> cost;
		g_table[src_Router]->insert(map<routerip_t, RTE>::value_type(dest_Router, RTE(dest_Router, cost)));
		g_table[dest_Router]->insert(map<routerip_t, RTE>::value_type(src_Router, RTE(src_Router, cost)));
		//중복이 안되는 한 삽입
		if (find(g_map[src_Router].begin(), g_map[src_Router].end(), dest_Router) == g_map[src_Router].end())
			g_map[src_Router].push_back(dest_Router);
		if (find(g_map[dest_Router].begin(), g_map[dest_Router].end(), src_Router) == g_map[dest_Router].end())
			g_map[dest_Router].push_back(src_Router);
		g_origin[src_Router][dest_Router] = cost;
		g_origin[dest_Router][src_Router] = cost;
	}

	//빈 곳 채우기
	for (int i = 0; i < g_size; ++i)
	{
		map<routerip_t, RTE>::iterator it;
		int c = 0;
		for (it = g_table[i]->begin(); it != g_table[i]->end(); ++it)
		{
			while (it->first > c)
			{
				g_table[i]->insert(it, map<routerip_t, RTE>::value_type(c++, RTE(-999, -999)));
			}
		}
		while (g_size > c)
		{
			g_table[i]->insert(it, map<routerip_t, RTE>::value_type(c++, RTE(-999, -999)));
		}
	}
	for (int i = 0; i < g_size; ++i)
	{
		xchgTable();
	}
	printRoutingTable(output);
	topology.close();
	return (true);
}

//char 문자열을 int로 바꾸어 주기
int     char_to_int(char* buf)
{
	int ans = 0;

	for (int i = 0; i < (int)strlen(buf); ++i)
	{
		int temp = buf[i] - '0';
		ans += ans * 10 + temp;
	}
	return (ans);
}

//패킷을 다른 라우터로 보내기
bool    send_packet(PACKET& pck)
{
	int pos = pck.src;
	//목적지 라우터에 도착하기 전까지 계속
	while (pos != pck.dest)
	{
		//현재의 테이블에 목적지를 가는 정보가 없는 경우
		map<routerip_t, RTE>::iterator it = g_table[pos]->find(pck.dest);
		if ((it->second.nexthop == -999))
		{
			return (0);
		}
		pos = it->second.nexthop;
		if (pos == pck.dest) break;
		pck.hop.push_back(pos);
	}
	return (1);
}

//메세지 내용읽어서 라우터에 보내기
void    readMsg(FILE* messages, ofstream& output)
{
	char msg[MAX_READ];
	char* token;

	while (fgets(msg, MAX_READ, messages))
	{
		msg[strlen(msg) - 1] = 0;
		PACKET pck;
		token = strtok(msg, LIMIT);
		pck.src = char_to_int(token);
		token = strtok(NULL, LIMIT);
		pck.dest = char_to_int(token);
		token = strtok(NULL, "");
		strncpy(pck.msg, token, MAX_READ);

		pck.cost = g_table[pck.src]->find(pck.dest)->second.cost;
		pck.hop.push_back(pck.src);

		if (pck.cost != -999 && send_packet(pck))
		{
			output << "from " << pck.src << " to " << pck.dest << " cost " << pck.cost << " hops ";
			//cout << "from " << pck.src << " to " << pck.dest << " cost " << pck.cost << " hops ";
			for (vector<int>::iterator it = pck.hop.begin(); it < pck.hop.end(); ++it)
			{
				output << *it << ' ';
				//cout << *it << ' ';
			}
			output << "message " << pck.msg << '\n';
			//cout << "message " << pck.msg << '\n';
		}
		else
		{
			output << "from " << pck.src << " to " << pck.dest << " cost infinite hops unreachable message " << pck.msg;
			//cout << "from " << pck.src << " to " << pck.dest << " cost infinite hops unreachable message " << pck.msg;
		}
	}
	output << '\n';
	//cout << '\n';
}

void    chgeTable(ifstream& changes)
{
	int src, dest, cost;
	map<routerip_t, RTE>::iterator s_it;
	map<routerip_t, RTE>::iterator d_it;

	changes >> src >> dest >> cost;
	//마지막까지 읽었다면 종료
	if (changes.eof()) return;

	s_it = g_table[src]->find(dest);
	d_it = g_table[dest]->find(src);
	//단순 변경 혹은 새로운 간선이 생기는 경우
	if (cost != -999)
	{
		vector<int>::iterator it = g_map[src].begin();

		//인자가 없는 경우
		if ((find(g_map[src].begin(), g_map[src].end(), dest) == g_map[src].end()) &&
			(find(g_map[dest].begin(), g_map[dest].end(), src) == g_map[dest].end()))
		{
			g_map[src].push_back(dest);
			g_map[dest].push_back(src);
			g_origin[src][dest] = cost;
			g_origin[dest][src] = cost;
		}
		//테이블 수정
		if (s_it->second.cost > cost)
		{
			s_it->second.cost = cost;
			s_it->second.nexthop = dest;
		}
		if (d_it->second.cost > cost)
		{
			d_it->second.cost = cost;
			d_it->second.nexthop = src;
		}
	}
	//간선이 끊긴 경우
	else {
		//인자가 있는지 확인
		vector<int>::iterator s_vit;
		vector<int>::iterator d_vit;
		if ((s_vit = find(g_map[src].begin(), g_map[src].end(), dest)) != g_map[src].end() &&
			(d_vit = find(g_map[dest].begin(), g_map[dest].end(), src)) != g_map[dest].end())
		{
			//벡터로 된 지도 수정
			g_map[src].erase(s_vit);
			g_map[dest].erase(d_vit);
			g_origin[src][dest] = -999;
			g_origin[dest][src] = -999;

			//우선 삭제된것의 테이블을 먼저 삭제
			map<routerip_t, RTE>::iterator s_it = g_table[src]->find(dest);
			map<routerip_t, RTE>::iterator d_it = g_table[dest]->find(src);

			//src, dest를 기준으로 src->dest, dest->src 인 것들을 모두 수정
			s_it->second.nexthop = -999;
			s_it->second.cost = -999;
			for (s_it = g_table[src]->begin(); s_it != g_table[src]->end(); ++s_it)
			{
				if (s_it->second.nexthop == dest)
				{
					s_it->second.nexthop = -999;
					s_it->second.cost = -999;
				}
			}

			d_it->second.nexthop = -999;
			d_it->second.cost = -999;
			for (d_it = g_table[dest]->begin(); d_it != g_table[dest]->end(); ++d_it)
			{
				if (d_it->second.nexthop == src)
				{
					d_it->second.nexthop = -999;
					d_it->second.cost = -999;
				}
			}
			//남아 있는 다른 노드들도 관리
			for (int i_src = 0; i_src < g_size; ++i_src)
			{
				//이미 처리된것은 건들지 않는다.
				if (i_src == src || i_src == dest) continue;
				//hop를 기준으로 처리
				map<routerip_t, RTE>::iterator it;
				for (it = g_table[i_src]->begin(); it != g_table[i_src]->end(); ++it)
				{
					//자기자신은 제회
					if (it->first == i_src) continue;
					//table의 dest와 관련이 있는 경우

					//src를 목적지로 삼는 테이블 내용인 경우
					if (g_origin[i_src][it->first] != -999 && it->first == src)
					{
						it->second.cost = g_origin[i_src][src];
						it->second.nexthop = src;
						g_table[src]->find(i_src)->second.nexthop = i_src;
						g_table[src]->find(i_src)->second.cost = g_origin[i_src][src];
					}
					//dest를 목적지로 삼는 테이블 내용인 경우
					else if (g_origin[i_src][it->first] != -999 && it->first == dest)
					{
						it->second.cost = g_origin[i_src][dest];
						it->second.nexthop = dest;
						g_table[dest]->find(i_src)->second.nexthop = i_src;
						g_table[dest]->find(i_src)->second.cost = g_origin[i_src][dest];
					}

					else if ((it->second.nexthop == src && it->first != src) || (it->second.nexthop == dest && it->first != dest)
						|| (it->first == src && g_origin[i_src][it->first] == -999) || (it->first == dest && g_origin[i_src][it->first] == -999)
						|| g_origin[it->first][i_src] == -999)
					{
						it->second.nexthop = -999;
						it->second.cost = -999;
					}
				}
			}
		}
	}
}

int     main(int argc, char* argv[])
{
	if (argc != 4)
	{
		printf("usage: distvec topologyfile messagesfile changesfile\n");
		return (0);
	}

	// 입력순서는 topology, message, change 순이라고 가정한다.
	ifstream topology(argv[1]);
	FILE* messages = fopen(argv[2], "r");
	//ifstream messages(argv[2]);
	ifstream changes(argv[3]);
	ofstream output("output_dv.txt");

	if (!topology || !messages || !changes)
	{
		printf("Error: open input file.\n");
		return (0);
	}
	printf("Complete. Output file written to output_dv.txt.\n");

	//routing table 제작
	initRouter(topology, output);
	readMsg(messages, output);
	fclose(messages);


	while (!changes.eof())
	{
		//change 파일 읽어놓기
		chgeTable(changes);
		if (changes.eof()) break;
		//테이블 바꾸기
		for (int i = 0; i < g_size; ++i)
		{
			xchgTable();
		}
		printRoutingTable(output);
		//printMap();
		messages = fopen(argv[2], "r");
		readMsg(messages, output);
		fclose(messages);
	}
}