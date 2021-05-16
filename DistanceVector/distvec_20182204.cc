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
#include <fstream>
#include <map>
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

    _packet() {}
    _packet(routerip_t src, routerip_t dest, char* msg)
        : src(src), dest(dest), cost(0)
    {
        strncpy(this->msg, msg, MAX_READ);
        hop.push_back(src);
    }
}PACKET;

//dest : key, nexthop,cost : value
map<routerip_t, RTE> *g_table[101];
vector<int> g_map[101];
int     g_size;
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
void    printRoutingTable(ofstream &output)
{
    for (int i = 0; i < g_size; ++i)
    {
        map<routerip_t, RTE>::iterator it;
        for (it = g_table[i]->begin(); it != g_table[i]->end(); ++it)
        {
            output << it->first << ' ' << it->second.nexthop << ' ' << it->second.cost << '\n';
        }
        output << '\n';
    }
}

//노드간 테이블 교환
void    excgTable(void)
{
    map<routerip_t, RTE>::iterator s_it;
    map<routerip_t, RTE>::iterator d_it;

    //전체 트리를 탐색
    for (int i = 0; i < g_size; ++i)
    {
        for (vector<int>::iterator v_it = g_map[i].begin(); v_it < g_map[i].end(); ++v_it)
        {
            //각자 서로의 위치로 갈때의 cost를 미리 저장 -> 반드시 존재
            int s_to_d_cost = (*(g_table[i])->find(*v_it)).second.cost;
            int d_to_s_cost = (*(g_table[*v_it])->find(i)).second.cost;

            //src를 dest 테이블을 기준으로 바꾸기
            for (d_it = g_table[*v_it]->begin(), s_it = g_table[i]->begin(); d_it != g_table[*v_it]->end() ; d_it++, s_it++)
            {
                //자기자신에 대한 내용인 경우
                if (d_it->first == *v_it) continue;
                if (s_it->first == i) continue;

                //dest도 목적지를 가기 위해 어떻게 가야하는지 모르는 경우
                if (d_it->second.nexthop == -999 && d_it->second.cost == -999) continue;
                
                // src 테이블 바꾸기
                if (s_it->second.cost == -999 || s_it->second.cost > s_to_d_cost + d_it->second.cost)
                {
                    s_it->second.nexthop = *v_it;
                    s_it->second.cost = s_to_d_cost + d_it->second.cost;
                }
            }
            //dest를 src 테이블을 기준으로 바꾸기
            for (d_it = g_table[*v_it]->begin(), s_it = g_table[i]->begin(); s_it != g_table[i]->end(); d_it++, s_it++)
            {
                //자기자신에 대한 내용인 경우
                if (d_it->first == *v_it) continue;
                if (s_it->first == i) continue;

                //src도 목적지를 가기 위해 어떻게 가야하는지 모르는 경우
                if (s_it->second.nexthop == -999 && s_it->second.cost == -999) continue;

                // dest 테이블 바꾸기
                if (d_it->second.cost == -999 || d_it->second.cost > d_to_s_cost + s_it->second.cost)
                {
                    d_it->second.nexthop = i;
                    d_it->second.cost = d_to_s_cost + s_it->second.cost;
                }
            }
        }
    }
}

//topology 읽기, graph 그리기
bool    initRouter(ifstream &topology, ofstream &output)
{
    routerip_t src_Router, dest_Router;
    cost_t cost;

    if (!topology) return (0);
    topology >> g_size;
    //자기자신 초기화
    for (int i = 0; i < g_size; ++i)
    {
        g_table[i] = new map<routerip_t, RTE>;
        g_table[i]->insert(map<routerip_t,RTE>::value_type(i,RTE(i,0)));
    }
    while (!topology.eof())
    {
        topology >> src_Router >> dest_Router >> cost;
        g_table[src_Router]->insert(map<routerip_t, RTE>::value_type(dest_Router, RTE(dest_Router, cost)));
        g_table[dest_Router]->insert(map<routerip_t, RTE>::value_type(src_Router, RTE(src_Router, cost)));
        g_map[src_Router].push_back(dest_Router);
        g_map[dest_Router].push_back(src_Router);
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
    //최소 두번은 돌리어 주어야 원하는 형태가 출력된다.
    excgTable();
    excgTable();
    cout << "===================" << '\n';
    printRoutingTable_win();
    printRoutingTable(output);
    topology.close();
    return (1);
}

//char 문자여을 int로 바꾸어 주기
int     char_to_int(char* buf)
{
    int ans = 0;

    for (int i = 0; i < strlen(buf); ++i)
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
void    readMsg(FILE* messages, ofstream &output)
{
    int src;
    int dest;
    char msg[MAX_READ];
    char* token;

    while (fgets(msg, MAX_READ, messages))
    {       
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
            output << "from " << pck.src << " to " << pck.dest << " cost " << pck.cost<< " hops ";
            cout << "from " << pck.src << " to " << pck.dest << " cost " << pck.cost << " hops ";
            for (vector<int>::iterator it = pck.hop.begin(); it < pck.hop.end(); ++it)
            {
                output << *it << ' ';
                cout << *it << ' ';
            }
            output << "message " << pck.msg;
            cout << "message " << pck.msg;
        }
        else
        {
            output << "from " << pck.src << " to " << pck.dest << " cost infinite hops unreachable message " << pck.msg;
            cout << "from " << pck.src << " to " << pck.dest << " cost infinite hops unreachable message " << pck.msg;
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
    
}
