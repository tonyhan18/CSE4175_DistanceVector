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


//�׽�Ʈ�� ���̺� ���
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

//���Ͽ� ���̺� ���
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

//��尣 ���̺� ��ȯ
void    xchgTable(void)
{
	map<routerip_t, RTE>::iterator s_it;
	map<routerip_t, RTE>::iterator d_it;

	//��ü Ʈ���� Ž��
	for (int src = 0; src < g_size; ++src)
	{
		for (vector<int>::iterator dest = g_map[src].begin(); dest < g_map[src].end(); ++dest)
		{
			//���� ������ ��ġ�� ������ cost�� �̸� ���� -> �ݵ�� ����
			int s_to_d_cost = g_origin[src][*dest];
			int d_to_s_cost = g_origin[*dest][src];

			//src�� dest ���̺��� �������� �ٲٱ�
			for (d_it = g_table[*dest]->begin(), s_it = g_table[src]->begin(); d_it != g_table[*dest]->end(); d_it++, s_it++)
			{
				//�ڱ��ڽſ� ���� ������ ���
				if (d_it->first == *dest) continue;
				if (s_it->first == src) continue;

				//dest�� �������� ���� ���� ��� �����ϴ��� �𸣴� ���
				if (d_it->second.nexthop == -999 && d_it->second.cost == -999) {}
				// src ���̺� �ٲٱ�
				// cost�� �����ų� �ش� ��带 1���� ��� ���� ��츸
				else if ((s_it->second.nexthop == -999 && s_it->second.cost == -999) || (s_it->second.cost >= s_to_d_cost + d_it->second.cost))
				{
					//cout<<"src : "<<src<<"="<<*dest<<' '<< s_to_d_cost + d_it->second.cost<<'\n';
					s_it->second.nexthop = *dest;
					s_it->second.cost = s_to_d_cost + d_it->second.cost;
				}

				//src�� �������� ���� ���� ��� �����ϴ��� �𸣴� ���
				if (s_it->second.nexthop == -999 && s_it->second.cost == -999) {}
				// dest ���̺� �ٲٱ�
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

//topology �б�, graph �׸���
bool    initRouter(ifstream& topology, ofstream& output)
{
	routerip_t src_Router, dest_Router;
	cost_t cost;

	if (!topology) return (false);
	topology >> g_size;
	//�ڱ��ڽ� �ʱ�ȭ
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
		//�ߺ��� �ȵǴ� �� ����
		if (find(g_map[src_Router].begin(), g_map[src_Router].end(), dest_Router) == g_map[src_Router].end())
			g_map[src_Router].push_back(dest_Router);
		if (find(g_map[dest_Router].begin(), g_map[dest_Router].end(), src_Router) == g_map[dest_Router].end())
			g_map[dest_Router].push_back(src_Router);
		g_origin[src_Router][dest_Router] = cost;
		g_origin[dest_Router][src_Router] = cost;
	}

	//�� �� ä���
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

//char ���ڿ��� int�� �ٲپ� �ֱ�
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

//��Ŷ�� �ٸ� ����ͷ� ������
bool    send_packet(PACKET& pck)
{
	int pos = pck.src;
	//������ ����Ϳ� �����ϱ� ������ ���
	while (pos != pck.dest)
	{
		//������ ���̺� �������� ���� ������ ���� ���
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

//�޼��� �����о ����Ϳ� ������
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
	//���������� �о��ٸ� ����
	if (changes.eof()) return;

	s_it = g_table[src]->find(dest);
	d_it = g_table[dest]->find(src);
	//�ܼ� ���� Ȥ�� ���ο� ������ ����� ���
	if (cost != -999)
	{
		vector<int>::iterator it = g_map[src].begin();

		//���ڰ� ���� ���
		if ((find(g_map[src].begin(), g_map[src].end(), dest) == g_map[src].end()) &&
			(find(g_map[dest].begin(), g_map[dest].end(), src) == g_map[dest].end()))
		{
			g_map[src].push_back(dest);
			g_map[dest].push_back(src);
			g_origin[src][dest] = cost;
			g_origin[dest][src] = cost;
		}
		//���̺� ����
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
	//������ ���� ���
	else {
		//���ڰ� �ִ��� Ȯ��
		vector<int>::iterator s_vit;
		vector<int>::iterator d_vit;
		if ((s_vit = find(g_map[src].begin(), g_map[src].end(), dest)) != g_map[src].end() &&
			(d_vit = find(g_map[dest].begin(), g_map[dest].end(), src)) != g_map[dest].end())
		{
			//���ͷ� �� ���� ����
			g_map[src].erase(s_vit);
			g_map[dest].erase(d_vit);
			g_origin[src][dest] = -999;
			g_origin[dest][src] = -999;

			//�켱 �����Ȱ��� ���̺��� ���� ����
			map<routerip_t, RTE>::iterator s_it = g_table[src]->find(dest);
			map<routerip_t, RTE>::iterator d_it = g_table[dest]->find(src);

			//src, dest�� �������� src->dest, dest->src �� �͵��� ��� ����
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
			//���� �ִ� �ٸ� ���鵵 ����
			for (int i_src = 0; i_src < g_size; ++i_src)
			{
				//�̹� ó���Ȱ��� �ǵ��� �ʴ´�.
				if (i_src == src || i_src == dest) continue;
				//hop�� �������� ó��
				map<routerip_t, RTE>::iterator it;
				for (it = g_table[i_src]->begin(); it != g_table[i_src]->end(); ++it)
				{
					//�ڱ��ڽ��� ��ȸ
					if (it->first == i_src) continue;
					//table�� dest�� ������ �ִ� ���

					//src�� �������� ��� ���̺� ������ ���
					if (g_origin[i_src][it->first] != -999 && it->first == src)
					{
						it->second.cost = g_origin[i_src][src];
						it->second.nexthop = src;
						g_table[src]->find(i_src)->second.nexthop = i_src;
						g_table[src]->find(i_src)->second.cost = g_origin[i_src][src];
					}
					//dest�� �������� ��� ���̺� ������ ���
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

	// �Է¼����� topology, message, change ���̶�� �����Ѵ�.
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

	//routing table ����
	initRouter(topology, output);
	readMsg(messages, output);
	fclose(messages);


	while (!changes.eof())
	{
		//change ���� �о����
		chgeTable(changes);
		if (changes.eof()) break;
		//���̺� �ٲٱ�
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