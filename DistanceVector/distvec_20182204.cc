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

//��尣 ���̺� ��ȯ
void    excgTable(void)
{
    map<routerip_t, RTE>::iterator s_it;
    map<routerip_t, RTE>::iterator d_it;

    //��ü Ʈ���� Ž��
    for (int i = 0; i < g_size; ++i)
    {
        for (vector<int>::iterator v_it = g_map[i].begin(); v_it < g_map[i].end(); ++v_it)
        {
            //���� ������ ��ġ�� ������ cost�� �̸� ���� -> �ݵ�� ����
            int s_to_d_cost = (*(g_table[i])->find(*v_it)).second.cost;
            int d_to_s_cost = (*(g_table[*v_it])->find(i)).second.cost;

            //src�� dest ���̺��� �������� �ٲٱ�
            for (d_it = g_table[*v_it]->begin(), s_it = g_table[i]->begin(); d_it != g_table[*v_it]->end() ; d_it++, s_it++)
            {
                //�ڱ��ڽſ� ���� ������ ���
                if (d_it->first == *v_it) continue;
                if (s_it->first == i) continue;

                //dest�� �������� ���� ���� ��� �����ϴ��� �𸣴� ���
                if (d_it->second.nexthop == -999 && d_it->second.cost == -999) continue;
                
                // src ���̺� �ٲٱ�
                if (s_it->second.cost == -999 || s_it->second.cost > s_to_d_cost + d_it->second.cost)
                {
                    s_it->second.nexthop = *v_it;
                    s_it->second.cost = s_to_d_cost + d_it->second.cost;
                }
            }
            //dest�� src ���̺��� �������� �ٲٱ�
            for (d_it = g_table[*v_it]->begin(), s_it = g_table[i]->begin(); s_it != g_table[i]->end(); d_it++, s_it++)
            {
                //�ڱ��ڽſ� ���� ������ ���
                if (d_it->first == *v_it) continue;
                if (s_it->first == i) continue;

                //src�� �������� ���� ���� ��� �����ϴ��� �𸣴� ���
                if (s_it->second.nexthop == -999 && s_it->second.cost == -999) continue;

                // dest ���̺� �ٲٱ�
                if (d_it->second.cost == -999 || d_it->second.cost > d_to_s_cost + s_it->second.cost)
                {
                    d_it->second.nexthop = i;
                    d_it->second.cost = d_to_s_cost + s_it->second.cost;
                }
            }
        }
    }
}

//topology �б�, graph �׸���
bool    initRouter(ifstream &topology, ofstream &output)
{
    routerip_t src_Router, dest_Router;
    cost_t cost;

    if (!topology) return (0);
    topology >> g_size;
    //�ڱ��ڽ� �ʱ�ȭ
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
    //�ּ� �ι��� ������ �־�� ���ϴ� ���°� ��µȴ�.
    excgTable();
    excgTable();
    cout << "===================" << '\n';
    printRoutingTable_win();
    printRoutingTable(output);
    topology.close();
    return (1);
}

//char ���ڿ��� int�� �ٲپ� �ֱ�
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
    
}
