#include <iostream>
#include <gmpxx.h>

#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <boost/heap/fibonacci_heap.hpp>

#include "server.hpp"

#include "ggm.h"
#include "crypto_stuff.hpp"
#include "data_structures.hpp"

#ifdef SEC_GDB_SIMPLE_MODE
#include "client.hpp"
#include "sec_compare.hpp"
#endif

using namespace std;

void Server::build_server_graph(string &F_1_s, string &P_s, string &P_t, Constrain &constrained_key, size_t ctr)
{
    unordered_set<string> accesed_vertecies;
    queue<string> q;

    accesed_vertecies.emplace(P_s);

    GGM ggm = {KEY_SIZE, MAX_GGM_DEPTH};
    Subkeys sub_key;

    ggm_derive(&ggm, &constrained_key, &sub_key);

    for(size_t i = 0; i < ctr; i++)
    {
        unsigned char UT_i[KEY_SIZE] = {0};
        unsigned char mask[KEY_SIZE] = {0};

        H_1((unsigned char*)F_1_s.c_str(), KEY_SIZE, (unsigned char*)sub_key.keys[i], KEY_SIZE, UT_i);

        H_2((unsigned char*)F_1_s.c_str(), KEY_SIZE, (unsigned char*)sub_key.keys[i], KEY_SIZE, mask);

        string& masked_ciph_i = this->D_e[string((char*)UT_i, KEY_SIZE)];

        unsigned char ciph_i[masked_ciph_i.length()];

        masking(masked_ciph_i.c_str(), masked_ciph_i.length(), mask, sizeof(mask), ciph_i);

        string P_v_i = string((char*)ciph_i, KEY_SIZE);
        string F_1_vi = string((char*)(ciph_i + KEY_SIZE), KEY_SIZE);

        string str_e_i = string((char*)(ciph_i + 2 * KEY_SIZE), sizeof(ciph_i) - 2 * KEY_SIZE);
        mpz_class e_i;
        set_mpz_raw(e_i.get_mpz_t(), str_e_i.size(), str_e_i.c_str());
        
        this->sever_graph.add_edge(P_s, P_v_i, e_i);

        q.push(P_v_i);
        this->D_key[P_v_i] = F_1_vi;
    }

    ggm_free_keys(&sub_key);

    while(!q.empty())
    {
        string P_u = q.front();
        q.pop();
        accesed_vertecies.emplace(P_u);

        if (P_u == P_t)
        {
            return;
        }

        // Look up P(u) and get all of T_i
        Constrain con;
        size_t ctr_inwhile;

#ifdef SEC_GDB_SIMPLE_MODE

        V_ITEM F_2_u_ctr = g_client.get_Dpv().at(P_u);
        ctr_inwhile =  F_2_u_ctr.ctr;
        if (ctr_inwhile != 0)
        {
            ggm_find_best_range_cover(&ggm, (char*)F_2_u_ctr.master_key.c_str(), 0, ctr_inwhile - 1, &con);
        }
        
#else
#endif
        
        if (ctr_inwhile != 0)
        {
            Subkeys sub_key_inwhile;
            ggm_derive(&ggm, &con, &sub_key_inwhile);

            string &F_1_u = this->D_key[P_u];
            
            for (size_t i = 0; i < F_2_u_ctr.ctr; i++)
            {
                unsigned char UT_i[KEY_SIZE];
                unsigned char mask[KEY_SIZE];
                H_1((unsigned char*)F_1_u.c_str(), KEY_SIZE, (unsigned char*)sub_key_inwhile.keys[i], KEY_SIZE, UT_i);

                H_2((unsigned char*)F_1_u.c_str(), KEY_SIZE, (unsigned char*)sub_key_inwhile.keys[i], KEY_SIZE, mask);
                
                if (this->D_e.find(string((char*)UT_i, KEY_SIZE)) == this->D_e.end())
                {
                    cout << "??????????\n";
                }
                string &masked_ciph_i = this->D_e[string((char*)UT_i, KEY_SIZE)];

                unsigned char ciph_i[masked_ciph_i.length()];

                masking(masked_ciph_i.c_str(), masked_ciph_i.length(), mask, sizeof(mask), ciph_i);

                string P_v_i = string((char *)ciph_i, KEY_SIZE);
                string F_1_vi = string((char *)(ciph_i + KEY_SIZE), KEY_SIZE);
                string str_e_i = string((char *)(ciph_i + 2 * KEY_SIZE), sizeof(ciph_i) - 2 * KEY_SIZE);
                mpz_class e_i;
                set_mpz_raw(e_i.get_mpz_t(), str_e_i.size(), str_e_i.c_str());

                this->sever_graph.add_edge(P_u, P_v_i, e_i);
                if (accesed_vertecies.find(P_v_i) == accesed_vertecies.end())
                {
                    q.push(P_v_i);
                    this->D_key[P_v_i] = F_1_vi;
                }
            }

            ggm_free_constrain(&con);
            ggm_free_keys(&sub_key_inwhile);
        }
    }
}

/*
bool Server::set_level(string &F_1_s, string &P_s, string &P_t, Constrain &constrained_key, size_t ctr)
{
    queue<string> q;
    this->level.clear();

    this->level[P_s] = 0;

    GGM ggm = {KEY_SIZE, MAX_GGM_DEPTH};
    Subkeys sub_key;

    ggm_derive(&ggm, &constrained_key, &sub_key);

    for(size_t i = 0; i < ctr; i++)
    {
        unsigned char UT_i[KEY_SIZE];
        unsigned char mask[KEY_SIZE];

        H_1((unsigned char*)F_1_s.c_str(), KEY_SIZE, (unsigned char*)sub_key.keys[i], KEY_SIZE, UT_i);

        H_2((unsigned char*)F_1_s.c_str(), KEY_SIZE, (unsigned char*)sub_key.keys[i], KEY_SIZE, mask);

        string& masked_ciph_i = D_e[string((char*)UT_i, KEY_SIZE)];

        unsigned char ciph_i[sizeof(masked_ciph_i.length())];

        masking(masked_ciph_i.c_str(), masked_ciph_i.length(), mask, sizeof(mask), ciph_i);

        string P_v_i = string((char*)ciph_i, KEY_SIZE);
        string F_1_vi = string((char*)(ciph_i + KEY_SIZE), KEY_SIZE);
        string str_e_i = string((char*)(ciph_i + 2 * KEY_SIZE), sizeof(ciph_i) - 2 * KEY_SIZE);
        mpz_class e_i;
        set_mpz_raw(e_i.get_mpz_t(), str_e_i.size(), str_e_i.c_str());

        // Pair <P(s), P(v_i)>
        ENC_E_ITEM pair_i = {P_s, P_v_i};

        if (cap_r.find(pair_i) == cap_r.end())
        {
            cap_r[pair_i] = e_i;
        }

        if (cap_r[pair_i] > 0) /////////////////////////
        {
            q.push(P_v_i);
            level[P_v_i] = level[P_s] + 1;
            D_key[P_v_i] = F_1_vi;
        }
    }

    ggm_free_keys(&sub_key);

    while(!q.empty())
    {
        string &P_u = q.front();
        q.pop();
        if (P_u == P_t)
        {
            return true;
        }

        // Look up P(u) and get all of T_i
        Constrain con;
        size_t ctr_inwhile;

#ifdef SEC_GDB_SIMPLE_MODE

        V_ITEM F_2_u_ctr = g_client.get_Dpv().at(P_u);
        ctr_inwhile =  F_2_u_ctr.ctr;
        if (ctr_inwhile != 0)
        {
            ggm_find_best_range_cover(&ggm, (char*)F_2_u_ctr.master_key.c_str(), 0, ctr_inwhile - 1, &con);
        }
        
#else
#endif
        if (ctr_inwhile != 0)
        {
            Subkeys sub_key_inwhile;
            ggm_derive(&ggm, &con, &sub_key_inwhile);

            string &F_1_u = D_key[P_u];
            
            for (size_t i = 0; i < F_2_u_ctr.ctr; i++)
            {
                unsigned char UT_i[KEY_SIZE];
                unsigned char mask[KEY_SIZE];
                H_1((unsigned char*)F_1_u.c_str(), KEY_SIZE, (unsigned char*)sub_key_inwhile.keys[i], KEY_SIZE, UT_i);

                H_2((unsigned char*)F_1_u.c_str(), KEY_SIZE, (unsigned char*)sub_key_inwhile.keys[i], KEY_SIZE, mask);
                
                string &masked_ciph_i = this->D_e[string((char*)UT_i, KEY_SIZE)];

                unsigned char ciph_i[masked_ciph_i.length()];

                masking(masked_ciph_i.c_str(), masked_ciph_i.length(), mask, sizeof(mask), ciph_i);

                string P_v_i = string((char *)ciph_i, KEY_SIZE);
                string F_1_vi = string((char *)(ciph_i + KEY_SIZE), KEY_SIZE);
                string str_e_i = string((char *)(ciph_i + 2 * KEY_SIZE), sizeof(ciph_i) - 2 * KEY_SIZE);
                mpz_class e_i;
                set_mpz_raw(e_i.get_mpz_t(), str_e_i.size(), str_e_i.c_str());
                
                ENC_E_ITEM pair_i = {P_s, P_v_i};

                if (cap_r.find(pair_i) == cap_r.end())
                {
                    cap_r[pair_i] = e_i;
                }

                if (this->level.find(P_v_i) == this->level.end() && cap_r[pair_i] > 0) /////////////////////
                {
                    this->level[P_v_i] = this->level[P_u] + 1;
                    q.push(P_v_i);
                }
            }

            ggm_free_constrain(&con);
            ggm_free_keys(&sub_key_inwhile);
        }
        
    }

    return false;
}
*/

bool Server::set_level(string &F_1_s, string &P_s, string &P_t, Constrain &constrained_key, size_t ctr)
{
    queue<string> q;

    this->level.clear();
    this->level[P_s] = 0;

    q.push(P_s);

    while(!q.empty())
    {
        string P_u = q.front();
        q.pop();

        if (P_u == P_t)
        {
            return true;
        }
        
        size_t current_level = this->level[P_u];
        for (Edge<mpz_class> &e : this->sever_graph.adjacency_list[this->sever_graph.vertices[P_u]])
        {
            if (this->level.find(e.dest.name) == this->level.end() 
                    && secure_compare_greater(this->pk, e.weight, this->zero))
            {
                q.push(e.dest.name);
                this->level.emplace(e.dest.name, current_level + 1);
            }
        }
    }

    return (this->level.find(P_t) == this->level.end()) ? false : true;
}

/*
mpz_class Server::augment_path(std::string &F_1_u, std::string &P_u, std::string &P_t, Constrain &constrain, size_t ctr, mpz_class gamma)
{
    mpz_class fb;

    string &F_1_t = this->D_key[P_t];

    if (F_1_u == F_1_t || gamma == 0)
    {
        return gamma;
    }

    GGM ggm = {KEY_SIZE, MAX_GGM_DEPTH};
    Subkeys sub_key;

    ggm_derive(&ggm, &constrain, &sub_key);

    for (size_t i = 0; i < ctr; i++)
    {
        unsigned char UT_i[KEY_SIZE];
        unsigned char mask[KEY_SIZE];

        H_1((unsigned char *)F_1_u.c_str(), KEY_SIZE, (unsigned char *)sub_key.keys[i], KEY_SIZE, UT_i);

        H_2((unsigned char *)F_1_u.c_str(), KEY_SIZE, (unsigned char *)sub_key.keys[i], KEY_SIZE, mask);

        string &masked_ciph_i = D_e[string((char *)UT_i, KEY_SIZE)];

        unsigned char ciph_i[sizeof(masked_ciph_i.length())];

        masking(masked_ciph_i.c_str(), masked_ciph_i.length(), mask, sizeof(mask), ciph_i);

        string P_v_i = string((char *)ciph_i, KEY_SIZE);
        string F_1_vi = string((char *)(ciph_i + KEY_SIZE), KEY_SIZE);
        string str_e_i = string((char *)(ciph_i + 2 * KEY_SIZE), sizeof(ciph_i) - 2 * KEY_SIZE);
        mpz_class e_i;
        set_mpz_raw(e_i.get_mpz_t(), str_e_i.size(), str_e_i.c_str());

        ENC_E_ITEM pair_i = {P_u, P_v_i};
        if (this->cap_r.find(pair_i) == this->cap_r.end())
        {
            this->cap_r[pair_i] = e_i;
        }

        // Here we generate constrained key for the recursive function.
        Constrain con;
        size_t ctr_sub;
#ifdef SEC_GDB_SIMPLE_MODE
        V_ITEM F_2_v_i_ctr = g_client.get_Dpv().at(P_v_i);
        ctr_sub = F_2_v_i_ctr.ctr;
        ggm_find_best_range_cover(&ggm, (char*)F_2_v_i_ctr.master_key.c_str(), 0, F_2_v_i_ctr.ctr - 1, &con);
#else
#endif
        if (ctr_sub != 0)
        {   
            if (this->level[P_v_i] == (this->level[P_u] + 1))
            {
                mpz_class r = this->augment_path(F_1_vi, P_v_i, P_t, con, ctr_sub, (gamma < this->cap_r[pair_i]) ? gamma : this->cap_r[pair_i]);
                if (r > 0)
                {
                    this->cap_r[pair_i] -= r;
                }
            }
        }
    }

    return fb;
}
*/

mpz_class Server::augment_path(string &F_1_u, string &P_u, string &P_t, Constrain &constrain, size_t ctr, mpz_class gamma)
{
    mpz_class all_cap(this->zero);
    if (P_u == P_t)
    {
        return gamma;
    }

    size_t cur_level = this->level[P_u];

    for (Edge<mpz_class> &e : this->sever_graph.adjacency_list[this->sever_graph.vertices[P_u]])
    {
        mpz_class local_cap;
        local_cap = JL_homo_sub(this->pk, gamma, all_cap);
        if (this->level.find(e.dest.name) != this->level.end() 
            && this->level[e.dest.name] == cur_level + 1)
        {
            mpz_class tmp = this->augment_path(this->D_key[e.dest.name], e.dest.name, P_t, constrain, e.dest.out_degree,
                                               secure_compare_less(this->pk, local_cap, e.weight) ? local_cap : e.weight);

            auto rev_edge = this->sever_graph.find_edge(e.dest, e.src);
            if (rev_edge == this->sever_graph.adjacency_list[e.dest].end()) // Cannot find the edge.
            {
                this->sever_graph.add_edge(e.dest.name, e.src.name, tmp);
            }
            else
            {
                rev_edge->weight = JL_homo_add(this->pk, rev_edge->weight, tmp);
            }

            all_cap = JL_homo_add(this->pk, all_cap, tmp);
            e.weight = JL_homo_sub(this->pk, e.weight, tmp);
        }
    }

    return all_cap;
}

mpz_class Server::query_flow(string &F_1_s, string &P_s, string &P_t, Constrain &constrained_key, size_t ctr)
{
    this->sever_graph.clear();
    this->D_key.clear();

    this->build_server_graph(F_1_s, P_s, P_t, constrained_key, ctr);

    mpz_class c_qf;
    JL_encryption(this->pk, 0, c_qf);

    mpz_class inf;
    JL_encryption(this->pk, INFIENITY, inf);

    while(set_level(F_1_s, P_s, P_t, constrained_key, ctr))
    {
        mpz_class tmp = augment_path(F_1_s, P_s, P_t, constrained_key, ctr, inf);
        c_qf = JL_homo_add(this->pk, c_qf, tmp);
    }

    return c_qf;
}

mpz_class Server::query_dist(std::string &F_1_s, std::string &P_s, std::string &P_t, Constrain &constrained_key, size_t ctr)
{
    CACHE_ITEM cache_tmp = {P_s, P_t};
    if (this->cache.find(cache_tmp) != this->cache.end())
    {
        g_s_use_cache++;
        return this->cache[cache_tmp];
    }

    mpz_class c_qd;
    JL_encryption(this->pk, 0, c_qd);

    FIBO_HEAP fh; // Initializing a fibonacci heap.

    unordered_map<string, FIBO_HEAP::handle_type> heap_handlers; // Use a hash table to access the vertex added into heap.
    unordered_set<string> chosen_vertices;

    this->xi.clear();
    this->D_key.clear();
    this->path.clear();

    // The reason of these steppes is for preventing source vertex from being chosen again.
    xi[P_s] = this->zero;
    path[P_s] = PATH_ITEM{P_s, this->zero};
    heap_handlers[P_s] = fh.push(HEAP_ITEM{P_s, this->zero});
    fh.pop();
    chosen_vertices.emplace(P_s); //Important!

    GGM ggm = {KEY_SIZE, MAX_GGM_DEPTH};
    Subkeys sub_keys;
    ggm_derive(&ggm, &constrained_key, &sub_keys);

    for (size_t i = 0; i < ctr; i++)
    {
        unsigned char UT_i[KEY_SIZE] = {0};
        unsigned char mask[KEY_SIZE] = {0};

        H_1((unsigned char*)F_1_s.c_str(), KEY_SIZE, (unsigned char*)sub_keys.keys[i], KEY_SIZE, UT_i);

        H_2((unsigned char*)F_1_s.c_str(), KEY_SIZE, (unsigned char*)sub_keys.keys[i], KEY_SIZE, mask);

        string& masked_ciph_i = this->D_e[string((char*)UT_i, KEY_SIZE)];

        unsigned char ciph_i[masked_ciph_i.length()];

        masking(masked_ciph_i.c_str(), masked_ciph_i.length(), mask, sizeof(mask), ciph_i);

        string P_v_i = string((char*)ciph_i, KEY_SIZE);
        string F_1_vi = string((char*)(ciph_i + KEY_SIZE), KEY_SIZE);

        string str_e_i = string((char*)(ciph_i + 2 * KEY_SIZE), sizeof(ciph_i) - 2 * KEY_SIZE);
        mpz_class e_i;
        set_mpz_raw(e_i.get_mpz_t(), str_e_i.size(), str_e_i.c_str());

        this->path[P_v_i] = PATH_ITEM{P_s, e_i};
        this->xi[P_v_i] = e_i;
        FIBO_HEAP::handle_type handler =  fh.push(HEAP_ITEM{P_v_i, xi[P_v_i]});
        heap_handlers[P_v_i] = handler;
        this->D_key[P_v_i] = F_1_vi;
    }

    ggm_free_keys(&sub_keys);
    g_s_ttt = 0;
    g_s_cccddd = 0;
    g_s_total_cnttt = ctr;
    while(!fh.empty())
    {
        HEAP_ITEM hi = fh.top();
        fh.pop();
        chosen_vertices.emplace(hi.vetex);

        string &P_u = hi.vetex;
        if (P_u == P_t)
        {
            this->cache.emplace(cache_tmp, xi[P_u]);
            string tmp = let_mpz_raw_to_str(xi[P_u].get_mpz_t());
            g_s_cache_size += 2 * KEY_SIZE + tmp.size();
            return xi[P_u];
        }
        size_t ctr_inwhile = 0;
        Constrain con;
#ifdef SEC_GDB_SIMPLE_MODE

        V_ITEM F_2_u_ctr = g_client.get_Dpv().at(P_u);
        ctr_inwhile =  F_2_u_ctr.ctr;
        if (ctr_inwhile != 0)
        {
            ggm_find_best_range_cover(&ggm, (char*)F_2_u_ctr.master_key.c_str(), 0, ctr_inwhile - 1, &con);
        }
        
#else
#endif
        g_s_total_cnttt += ctr_inwhile;
        if (ctr_inwhile != 0)
        {
            Subkeys sub_key_inwhile;
            ggm_derive(&ggm, &con, &sub_key_inwhile);

            string &F_1_u = D_key[P_u];
            
            for (size_t i = 0; i < F_2_u_ctr.ctr; i++)
            {
                unsigned char UT_i[KEY_SIZE];
                unsigned char mask[KEY_SIZE];
                H_1((unsigned char*)F_1_u.c_str(), KEY_SIZE, (unsigned char*)sub_key_inwhile.keys[i], KEY_SIZE, UT_i);

                H_2((unsigned char*)F_1_u.c_str(), KEY_SIZE, (unsigned char*)sub_key_inwhile.keys[i], KEY_SIZE, mask);
                
                string &masked_ciph_i = this->D_e[string((char*)UT_i, KEY_SIZE)];

                unsigned char ciph_i[masked_ciph_i.length()];

                masking(masked_ciph_i.c_str(), masked_ciph_i.length(), mask, sizeof(mask), ciph_i);

                string P_v_i = string((char *)ciph_i, KEY_SIZE);
                string F_1_vi = string((char *)(ciph_i + KEY_SIZE), KEY_SIZE);
                string str_e_i = string((char *)(ciph_i + 2 * KEY_SIZE), sizeof(ciph_i) - 2 * KEY_SIZE);
                mpz_class e_i;
                set_mpz_raw(e_i.get_mpz_t(), str_e_i.size(), str_e_i.c_str());
                
                // If cannot find P_v_i in xi, the latter condition may cause error.
                // Also the first condition checks whether P_v_i is accessed.
                // if (xi.find(P_v_i) == xi.end() || xi[P_u] + e_i < xi[P_v_i]) 
                mpz_class tmp(JL_homo_add(this->pk, xi[P_u], e_i));
                if (xi.find(P_v_i) == xi.end() || secure_compare_less(this->pk, tmp, xi[P_v_i])) 
                {
                    xi[P_v_i] = tmp;
                    path[P_v_i] = PATH_ITEM{P_u, e_i};
                }

                if (heap_handlers.find(P_v_i) == heap_handlers.end())
                {
                    FIBO_HEAP::handle_type handler = fh.push(HEAP_ITEM{P_v_i, xi[P_v_i]});
                    heap_handlers[P_v_i] = handler;
                }
                else
                {
                    // Check if the P_v_i has been chosen, namely the distance of 
                    // P_v_i is shortest.
                    if (chosen_vertices.find(P_v_i) == chosen_vertices.end()) 
                    {
                        fh.update(heap_handlers[P_v_i], HEAP_ITEM{P_v_i, xi[P_v_i]});
                    }
                }

                D_key[P_v_i] = F_1_vi;
                g_s_cccddd ++;
            }
            
            ggm_free_keys(&sub_key_inwhile);
            ggm_free_constrain(&con);
        }
        g_s_ttt++;
    }
    this->cache.emplace(cache_tmp, c_qd);
    string tmp = let_mpz_raw_to_str(c_qd.get_mpz_t());
    g_s_cache_size += 2 * KEY_SIZE + tmp.size();
    return c_qd;
}



Server::Server(const unordered_map<string, string> &de, const PK &pk) : D_e(de), pk(pk), level(), D_key(), xi(), path(), sever_graph(), zero(), cache()
{
    JL_encryption(this->pk, 0, this->zero);
}

Server::~Server()
{
}