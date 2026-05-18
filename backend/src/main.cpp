#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "Ws2_32.lib")
    #endif
    #define close closesocket
    typedef int socklen_t;
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdlib>

#include "graph.hpp"

using namespace std;

static string url_decode(const string &s) {
    string out;
    char a, b;
    for (size_t i = 0; i < s.size(); ++i) {
        if ((s[i] == '%') && i + 2 < s.size() && isxdigit(a = s[i+1]) && isxdigit(b = s[i+2])) {
            istringstream iss(s.substr(i+1,2)); int x; iss >> hex >> x;
            out += static_cast<char>(x);
            i += 2;
        } else if (s[i] == '+') out += ' ';
        else out += s[i];
    }
    return out;
}

static map<string,string> parse_query(const string &qs) {
    map<string,string> m;
    size_t i = 0;
    while (i < qs.size()) {
        size_t j = qs.find('=', i);
        if (j == string::npos) break;
        string k = qs.substr(i, j - i);
        size_t a = qs.find('&', j+1);
        string v = (a == string::npos) ? qs.substr(j+1) : qs.substr(j+1, a - (j+1));
        m[k] = url_decode(v);
        if (a == string::npos) break;
        i = a + 1;
    }
    return m;
}

static string read_request(int client) {
    string req;
    char buf[4096];
    int n = recv(client, buf, sizeof(buf)-1, 0);
    if (n <= 0) return req;
    buf[n] = '\0';
    req = string(buf);
    size_t pos = req.find("Content-Length:");
    if (pos != string::npos) {
        pos += strlen("Content-Length:");
        size_t eol = req.find('\r', pos);
        string lenstr = req.substr(pos, eol - pos);
        int len = atoi(lenstr.c_str());
        size_t header_end = req.find("\r\n\r\n");
        if (header_end != string::npos) {
            size_t body_len = req.size() - (header_end + 4);
            while ((int)body_len < len) {
                int r = recv(client, buf, sizeof(buf)-1, 0);
                if (r <= 0) break;
                buf[r] = '\0';
                req.append(buf, r);
                body_len = req.size() - (header_end + 4);
            }
        }
    }
    return req;
}

static string json_escape(const string &s) {
    string out;
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            default: out += c; break;
        }
    }
    return out;
}

static bool parse_int_strict(const string &s, int &out) {
    if (s.empty()) return false;
    char *endptr = nullptr;
    errno = 0;
    long v = strtol(s.c_str(), &endptr, 10);
    if (errno != 0) return false;
    if (endptr == s.c_str() || *endptr != '\0') return false;
    if (v <= 0 || v > INT_MAX) return false;
    out = (int)v;
    return true;
}

static bool parse_double_strict(const string &s, double &out) {
    if (s.empty()) return false;
    char *endptr = nullptr;
    errno = 0;
    double v = strtod(s.c_str(), &endptr);
    if (errno != 0) return false;
    if (endptr == s.c_str() || *endptr != '\0') return false;
    if (!(v > 0.0)) return false;
    out = v;
    return true;
}

static bool parse_int64_strict(const string &s, int64_t &out) {
    if (s.empty()) return false;
    char *endptr = nullptr;
    errno = 0;
    long long v = strtoll(s.c_str(), &endptr, 10);
    if (errno != 0) return false;
    if (endptr == s.c_str() || *endptr != '\0') return false;
    if (v <= 0) return false;
    out = static_cast<int64_t>(v);
    return true;
}

int main() {
    #ifdef _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            cerr << "WSAStartup failed" << endl;
            return 1;
        }
    #endif

    Graph G;

    int port = 8080;
    if (const char *port_env = getenv("PORT")) {
        int parsed_port = atoi(port_env);
        if (parsed_port > 0 && parsed_port <= 65535) {
            port = parsed_port;
        }
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0 || server_fd == -1) { 
        #ifdef _WIN32
            cerr << "socket failed with error: " << WSAGetLastError() << endl;
        #else
            perror("socket");
        #endif
        return 1; 
    }
    int opt = 1;
    #ifdef _WIN32
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
            cerr << "setsockopt failed" << endl;
            return 1;
        }
    #else
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { 
        #ifdef _WIN32
            cerr << "bind failed with error: " << WSAGetLastError() << endl;
        #else
            perror("bind");
        #endif
        return 1; 
    }
    
    if (listen(server_fd, 10) < 0) { 
        #ifdef _WIN32
            cerr << "listen failed with error: " << WSAGetLastError() << endl;
        #else
            perror("listen");
        #endif
        return 1; 
    }

    cout << "GraphAnalyse engine listening on http://0.0.0.0:" << port << endl;

    while (true) {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0) continue;
        string req = read_request(client);
        if (req.empty()) { close(client); continue; }

        istringstream iss(req);
        string method, pathq, version;
        iss >> method >> pathq >> version;

        string path = pathq;
        string query;
        size_t qm = pathq.find('?');
        if (qm != string::npos) { path = pathq.substr(0, qm); query = pathq.substr(qm+1); }

    string body;
    size_t header_end = req.find("\r\n\r\n");
    if (header_end != string::npos) body = req.substr(header_end+4);

    cerr << "REQ: " << method << " " << path << " body_len=" << body.size() << endl;

        string response_body;
        string status = "200 OK";

        try {
            if (method == "OPTIONS") {
                status = "204 No Content";
                response_body.clear();
            }
            else if (method == "GET" && path == "/health") {
                response_body = "{\"status\":\"ok\"}";
            }
            else if (method == "POST" && path == "/user") {
                auto params = parse_query(body);
                string username = params.count("username") ? params["username"] : "";
                if (username.empty()) { status = "400 Bad Request"; response_body = "{\"error\":\"missing username\"}"; }
                else {
                    int id = G.add_user(username);
                    if (id < 0) {
                        status = "409 Conflict";
                        response_body = "{\"error\":\"username already exists\"}";
                    } else {
                        G.recompute_analytics();
                        response_body = "{\"user_id\":" + to_string(id) + ",\"username\":\"" + json_escape(username) + "\"}";
                    }
                }
            }
            else if (method == "POST" && path == "/post") {
                auto params = parse_query(body);
                string user_s = params.count("user_id") ? params["user_id"] : "";
                int user_id = 0;
                if (!parse_int_strict(user_s, user_id)) {
                    status = "400 Bad Request"; response_body = "{\"error\":\"invalid user_id\"}";
                } else {
                    string content = params.count("content") ? params["content"] : "";
                    if (content.empty()) { status = "400 Bad Request"; response_body = "{\"error\":\"missing content\"}"; }
                    else {
                        bool flagged = G.moderate_content(content);
                        if (flagged) {
                            status = "400 Bad Request";
                            response_body = "{\"error\": \"Post flagged for containing 'vulgar language'.\"}";
                        } else {
                            int pid = G.add_post(user_id, content);
                            if (pid < 0) {
                                status = "400 Bad Request";
                                response_body = "{\"error\":\"user not found\"}";
                            } else {
                                G.recompute_analytics();
                                response_body = "{\"post_id\":" + to_string(pid) + "}";
                            }
                        }
                    }
                }
            }
            else if (method == "POST" && path == "/interaction") {
                auto params = parse_query(body);
                string type = params.count("type") ? params["type"] : "";
                string user_s = params.count("user_id") ? params["user_id"] : "";
                string target_s = params.count("target_id") ? params["target_id"] : "";
                int user_id = 0, target = 0;
                double weight = 0.0;
                int64_t timestamp = 0;
                if (!parse_int_strict(user_s, user_id) || !parse_int_strict(target_s, target)) {
                    status = "400 Bad Request"; response_body = "{\"error\":\"invalid user_id or target_id\"}";
                } else if (params.count("weight") && !parse_double_strict(params["weight"], weight)) {
                    status = "400 Bad Request"; response_body = "{\"error\":\"invalid weight\"}";
                } else if (params.count("timestamp") && !parse_int64_strict(params["timestamp"], timestamp)) {
                    status = "400 Bad Request"; response_body = "{\"error\":\"invalid timestamp\"}";
                } else {
                    bool interaction_applied = false;
                    if (type == "follow") {
                        bool ok = G.add_follow(user_id, target);
                        if (!ok) {
                            status = "400 Bad Request";
                            response_body = "{\"error\":\"invalid follow request\"}";
                        } else {
                            interaction_applied = true;
                        }
                    }
                    else if (type == "like") {
                        bool ok = G.add_like(user_id, target, weight > 0.0 ? weight : 3.0, timestamp);
                        if (!ok) {
                            status = "400 Bad Request";
                            response_body = "{\"error\":\"user must follow author to like their posts\"}";
                        } else {
                            interaction_applied = true;
                        }
                    }
                    else if (type == "view") {
                        bool ok = G.add_view(user_id, target, weight > 0.0 ? weight : 1.0, timestamp);
                        if (!ok) {
                            status = "400 Bad Request";
                            response_body = "{\"error\":\"invalid view request\"}";
                        } else {
                            interaction_applied = true;
                        }
                    }
                    else { status = "400 Bad Request"; response_body = "{\"error\":\"bad interaction type\"}"; }
                    if (interaction_applied) G.recompute_analytics();
                    if (response_body.empty()) response_body = "{\"status\":\"ok\"}";
                }
            }
                else if (method == "POST" && path == "/user/delete") {
                    auto params = parse_query(body);
                    string user_s = params.count("user_id") ? params["user_id"] : "";
                    int uid = 0;
                    if (!parse_int_strict(user_s, uid)) {
                        status = "400 Bad Request"; response_body = "{\"error\":\"invalid user_id\"}";
                    } else {
                        bool ok = G.delete_user(uid);
                        if (!ok) { status = "400 Bad Request"; response_body = "{\"error\":\"user not found\"}"; }
                        else { G.recompute_analytics(); response_body = "{\"status\":\"deleted\"}"; }
                    }
                }
            else if (method == "GET" && path == "/users-list") {
                auto q = parse_query(query);
                int page = 1, limit = 10;
                if (q.count("page")) parse_int_strict(q["page"], page);
                if (q.count("limit")) parse_int_strict(q["limit"], limit);
                auto list = G.users_list(page, limit);
                ostringstream os; os << "[";
                for (size_t i = 0; i < list.size(); ++i) {
                    if (i) os << ",";
                    os << "{\"user_id\":" << list[i].first << ",\"username\":\"" << json_escape(list[i].second) << "\"}";
                }
                os << "]";
                response_body = os.str();
            }
            else if (method == "GET" && path == "/users/ranked") {
                auto q = parse_query(query);
                int page = 1, limit = 5;
                if (q.count("page")) parse_int_strict(q["page"], page);
                if (q.count("limit")) parse_int_strict(q["limit"], limit);
                auto ranked = G.get_ranked(page, limit);
                ostringstream os; os << "[";
                for (size_t i = 0; i < ranked.size(); ++i) {
                    if (i) os << ",";
                    int uid = ranked[i].first;
                    auto m = G.get_user_metrics(uid);
                    os << "{\"user_id\":" << uid << ",\"username\":\"" << json_escape(ranked[i].second) << "\",\"score\":" << ranked[i].third
                       << ",\"followers\": " << m.followers << ",\"followings\": " << m.followings
                       << ",\"total_likes\": " << m.total_likes << ",\"posts\": " << m.posts << "}";
                }
                os << "]";
                response_body = os.str();
            }

            else if (method == "GET" && path.rfind("/user/metrics/",0) == 0) {
                int uid = 0;
                if (!parse_int_strict(path.substr(strlen("/user/metrics/")), uid)) {
                    status = "400 Bad Request";
                    response_body = "{\"error\":\"invalid user_id\"}";
                } else {
                    auto m = G.get_user_metrics(uid);
                    ostringstream os;
                    os << "{\"user_id\":" << uid << ",\"followers\": " << m.followers << ",\"followings\": " << m.followings
                       << ",\"total_likes\": " << m.total_likes << ",\"posts\": " << m.posts << ",\"score\": " << m.score << "}";
                    response_body = os.str();
                }
            }

            else if (method == "GET" && path.rfind("/user/followers/",0) == 0) {
                int uid = 0;
                if (!parse_int_strict(path.substr(strlen("/user/followers/")), uid)) {
                    status = "400 Bad Request";
                    response_body = "{\"error\":\"invalid user_id\"}";
                } else {
                    auto v = G.get_followers(uid);
                    ostringstream os; os << "[";
                    for (size_t i = 0; i < v.size(); ++i) { if (i) os << ","; os << v[i]; }
                    os << "]"; response_body = os.str();
                }
            }

            else if (method == "GET" && path.rfind("/user/followings/",0) == 0) {
                int uid = 0;
                if (!parse_int_strict(path.substr(strlen("/user/followings/")), uid)) {
                    status = "400 Bad Request";
                    response_body = "{\"error\":\"invalid user_id\"}";
                } else {
                    auto v = G.get_followings(uid);
                    ostringstream os; os << "[";
                    for (size_t i = 0; i < v.size(); ++i) { if (i) os << ","; os << v[i]; }
                    os << "]"; response_body = os.str();
                }
            }

            else if (method == "GET" && path.rfind("/user/likedposts/",0) == 0) {
                int uid = 0;
                if (!parse_int_strict(path.substr(strlen("/user/likedposts/")), uid)) {
                    status = "400 Bad Request";
                    response_body = "{\"error\":\"invalid user_id\"}";
                } else {
                    auto v = G.get_liked_posts(uid);
                    ostringstream os; os << "[";
                    for (size_t i = 0; i < v.size(); ++i) { if (i) os << ","; os << v[i]; }
                    os << "]"; response_body = os.str();
                }
            }
            else if (method == "GET" && path.rfind("/post/metrics/",0) == 0) {
                int pid = 0;
                if (!parse_int_strict(path.substr(strlen("/post/metrics/")), pid)) {
                    status = "400 Bad Request";
                    response_body = "{\"error\":\"invalid post_id\"}";
                } else {
                    auto m = G.get_post_metrics(pid);
                    ostringstream os;
                    os << "{\"post_id\":" << pid << ",\"likes\":" << m.likes
                       << ",\"unique_views\":" << m.unique_views
                       << ",\"score\":" << m.score
                       << ",\"interaction_weight\":" << m.interaction_weight << "}";
                    response_body = os.str();
                }
            }
            else if (method == "GET" && path.rfind("/post/unique-views/",0) == 0) {
                int pid = 0;
                if (!parse_int_strict(path.substr(strlen("/post/unique-views/")), pid)) {
                    status = "400 Bad Request";
                    response_body = "{\"error\":\"invalid post_id\"}";
                } else {
                    auto m = G.get_post_metrics(pid);
                    response_body = "{\"post_id\":" + to_string(pid) + ",\"unique_views\":" + to_string(m.unique_views) + "}";
                }
            }
            else if (method == "GET" && path == "/posts/top10") {
                auto tops = G.top_posts();
                ostringstream os; os << "[";
                for (size_t i = 0; i < tops.size(); ++i) {
                    if (i) os << ",";
                    os << "{\"post_id\":" << tops[i].post_id << ",\"user_id\":" << tops[i].user_id
                       << ",\"likes\":" << tops[i].likes
                       << ",\"unique_views\":" << tops[i].unique_views
                       << ",\"score\":" << tops[i].score
                       << ",\"interaction_weight\":" << tops[i].interaction_weight
                       << ",\"content\":\"" << json_escape(tops[i].content) << "\"}";
                }
                os << "]";
                response_body = os.str();
            }
            else if (method == "GET" && path == "/posts/all") {
                auto all = G.all_posts();
                ostringstream os; os << "[";
                for (size_t i = 0; i < all.size(); ++i) {
                    if (i) os << ",";
                    os << "{\"post_id\":" << all[i].post_id << ",\"user_id\":" << all[i].user_id
                       << ",\"likes\":" << all[i].likes
                       << ",\"unique_views\":" << all[i].unique_views
                       << ",\"score\":" << all[i].score
                       << ",\"interaction_weight\":" << all[i].interaction_weight
                       << ",\"content\":\"" << json_escape(all[i].content) << "\"}";
                }
                os << "]";
                response_body = os.str();
            }
            else if (method == "POST" && path == "/post/delete") {
                auto params = parse_query(body);
                string post_s = params.count("post_id") ? params["post_id"] : "";
                int pid = 0;
                if (!parse_int_strict(post_s, pid)) { status = "400 Bad Request"; response_body = "{\"error\":\"invalid post_id\"}"; }
                else {
                    bool ok = G.delete_post(pid);
                    if (!ok) { status = "400 Bad Request"; response_body = "{\"error\":\"post not found\"}"; }
                    else { G.recompute_analytics(); response_body = "{\"status\":\"deleted\"}"; }
                }
            }
            else if (method == "GET" && path == "/path") {
                auto q = parse_query(query);
                int u1 = 0, u2 = 0;
                if (!q.count("u1") || !q.count("u2") || !parse_int_strict(q["u1"], u1) || !parse_int_strict(q["u2"], u2)) {
                    status = "400 Bad Request"; response_body = "{\"error\":\"invalid u1 or u2\"}";
                } else {
                    auto p = G.bfs_path(u1, u2);
                    ostringstream os; os << "{\"path\": [";
                    for (size_t i = 0; i < p.size(); ++i) { if (i) os << ","; os << p[i]; }
                    os << "]}";
                    response_body = os.str();
                }
            }
            else if (method == "GET" && path == "/recommendations") {
                auto q = parse_query(query);
                int u = 0;
                if (!q.count("u") || !parse_int_strict(q["u"], u)) {
                    status = "400 Bad Request"; response_body = "{\"error\":\"invalid u\"}";
                } else {
                    auto rec = G.recommendations(u);
                    ostringstream os; os << "[";
                    for (size_t i = 0; i < rec.size(); ++i) { if (i) os << ","; os << rec[i]; }
                    os << "]";
                    response_body = os.str();
                }
            }
            else if (method == "GET" && path == "/communities") {
                auto comm = G.communities();
                ostringstream os; os << "[";
                bool firstC = true;
                for (auto &c : comm) {
                    if (!firstC) os << ",";
                    firstC = false;
                    os << "{\"community_id\":" << c.first << ",\"members\":[";
                    for (size_t i = 0; i < c.second.size(); ++i) { if (i) os << ","; os << c.second[i]; }
                    os << "]}";
                }
                os << "]";
                response_body = os.str();
            }
            else if (method == "GET" && path.rfind("/community/",0) == 0) {
                int cid = 0;
                if (!parse_int_strict(path.substr(strlen("/community/")), cid)) {
                    status = "400 Bad Request";
                    response_body = "{\"error\":\"invalid community_id\"}";
                } else {
                    auto members = G.community_members(cid);
                    ostringstream os; os << "[";
                    for (size_t i = 0; i < members.size(); ++i) { if (i) os << ","; os << members[i]; }
                    os << "]";
                    response_body = os.str();
                }
            }
            else if (method == "GET" && path == "/search") {
                auto q = parse_query(query);
                string qv = q.count("q") ? q["q"] : "";
                auto ids = G.search_posts(qv);
                auto all_posts = G.all_posts();
                map<int, PostInfo> posts_by_id;
                for (const auto &post : all_posts) posts_by_id[post.post_id] = post;
                ostringstream os; os << "[";
                bool first = true;
                for (int id : ids) {
                    auto it = posts_by_id.find(id);
                    if (it == posts_by_id.end()) continue;
                    const auto &post = it->second;
                    if (!first) os << ",";
                    first = false;
                    os << "{\"post_id\":" << post.post_id
                       << ",\"user_id\":" << post.user_id
                       << ",\"likes\":" << post.likes
                       << ",\"unique_views\":" << post.unique_views
                       << ",\"score\":" << post.score
                       << ",\"interaction_weight\":" << post.interaction_weight
                       << ",\"content\":\"" << json_escape(post.content) << "\"}";
                }
                os << "]";
                response_body = os.str();
            }
            else if (method == "GET" && path == "/autocomplete/users") {
                // Trie-based autocomplete for usernames only
                auto q = parse_query(query);
                string pref = q.count("prefix") ? q["prefix"] : "";
                auto res = G.autocomplete_users(pref);
                ostringstream os; os << "[";
                for (size_t i = 0; i < res.size(); ++i) { if (i) os << ","; os << "\"" << json_escape(res[i]) << "\""; }
                os << "]";
                response_body = os.str();
            }
            else if (method == "GET" && path == "/autocomplete/posts") {
                // Trie-based autocomplete for post content keywords
                auto q = parse_query(query);
                string pref = q.count("prefix") ? q["prefix"] : "";
                auto res = G.autocomplete_posts(pref);
                ostringstream os; os << "[";
                for (size_t i = 0; i < res.size(); ++i) { if (i) os << ","; os << "\"" << json_escape(res[i]) << "\""; }
                os << "]";
                response_body = os.str();
            }
            else if (method == "GET" && path == "/search/aho") {
                // Aho-Corasick based pattern search in posts
                auto q = parse_query(query);
                string pattern = q.count("pattern") ? q["pattern"] : "";
                auto res = G.search_posts_aho(pattern);
                ostringstream os; os << "[";
                for (size_t i = 0; i < res.size(); ++i) { if (i) os << ","; os << res[i]; }
                os << "]";
                response_body = os.str();
            }
            else {
                status = "404 Not Found";
                response_body = "{\"error\":\"not found\"}";
            }
        } catch (const exception &e) {
            status = "500 Internal Server Error";
            response_body = string("{\"error\":\"") + json_escape(e.what()) + "\"}";
        }

        ostringstream resp;
        resp << "HTTP/1.1 " << status << "\r\n";
        resp << "Content-Type: application/json\r\n";
        resp << "Access-Control-Allow-Origin: *\r\n";
        resp << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        resp << "Access-Control-Allow-Headers: Content-Type\r\n";
        resp << "Content-Length: " << response_body.size() << "\r\n";
        resp << "Connection: close\r\n";
        resp << "\r\n";
        resp << response_body;

        string out = resp.str();
        send(client, out.c_str(), out.size(), 0);
        close(client);
    }

    close(server_fd);
    #ifdef _WIN32
        WSACleanup();
    #endif
    return 0;
}
