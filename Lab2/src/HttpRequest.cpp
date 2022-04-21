/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 
#include "HttpRequest.h"
#include "server.h"
using namespace std;


const unordered_set<string> HttpRequest::DEFAULT_HTML{
            "/index" };

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

void HttpRequest::Init() {
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpRequest::IsKeepAlive() const {
    if(header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool HttpRequest::parse(Buffer& buff) {
    const char CRLF[] = "\r\n";
    if(buff.ReadableBytes() <= 0) {
        return false;
    }
    while(buff.ReadableBytes() && state_ != FINISH) {
        const char* lineEnd = search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buff.Peek(), lineEnd);
        switch(state_)
        {
        case REQUEST_LINE:
            if(!ParseRequestLine_(line)) {
                return false;
            }
            ParsePath_();
            break;    
        case HEADERS:
            ParseHeader_(line);
            if(buff.ReadableBytes() <= 2) {
                state_ = FINISH;
            }
            break;
        case BODY:
            ParseBody_(line);
            break;
        default:
            break;
        }
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2);
    }
    //LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    if(DEBUG)
        printf("[%s], [%s], [%s]\n", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

//添加get参数解析,懒得写url解析.jpg
void HttpRequest::ParsePath_() {
    string get = path_;
    int pos = path_.find('?');
    if(pos != path_.npos)
        path_ = path_.substr(0,pos);//front '?'
    get = get.substr(pos+1);//behind '?'
    if(DEBUG)
        printf("DEBUG: Parsing GET (%s)\n",path_.c_str());
    ParseFromUrlencoded_(get);

    if(path_ == "/") {
        path_ = "/index.html"; 
    }
    else if(path_ == "/api/check"){
        path_ = "/text/data.txt";
    }
    else if(path_ == "/api/echo"){
        path_ = "/text/echo.txt";
    }
    else {
        for(auto &item: DEFAULT_HTML) {
            if(item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

bool HttpRequest::ParseRequestLine_(const string& line) {
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {   
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS;
        return true;
    }
    //LOG_ERROR("RequestLine Error");
    printf("RequestLine Error\n");
    return false;
}

void HttpRequest::ParseHeader_(const string& line) {
    regex patten("^([^:]*): ?(.*)$");
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        header_[subMatch[1]] = subMatch[2];
    }
    else {
        state_ = BODY;
    }
}

void HttpRequest::ParseBody_(const string& line) {
    body_ = line;
    ParsePost_();
    state_ = FINISH;
    if(DEBUG)
        //LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
        printf("DEBUG: Body:%s, len:%d\n", line.c_str(), line.size());
}

int HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

void HttpRequest::ParsePost_() {
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded_(body_);
        if(DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            if(DEBUG)
                //LOG_DEBUG("Tag:%d", tag);
                printf("DEBUG: Tag:%d\n", tag);
            if(tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                path_ = "/welcome.html";
            }
        }
    }   
}

//修改参数以兼容get
void HttpRequest::ParseFromUrlencoded_(string& data) {
    if(data.size() == 0) { return; }

    string key, value;
    int num = 0;
    int n = data.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = data[i];
        switch (ch) {
        case '=':
            key = data.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            data[i] = ' ';
            break;
        case '%':
            num = ConverHex(data[i + 1]) * 16 + ConverHex(data[i + 2]);
            data[i + 2] = num % 10 + '0';
            data[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = data.substr(j, i - j);
            j = i + 1;
            post_[key] = value;
            if(DEBUG)
                //LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                printf("DEBUG: %s = %s\n", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(post_.count(key) == 0 && j < i) {
        value = data.substr(j, i - j);
        post_[key] = value;
    }
}

std::string HttpRequest::path() const{
    return path_;
}

std::string& HttpRequest::path(){
    return path_;
}
std::string HttpRequest::method() const {
    return method_;
}

std::string HttpRequest::version() const {
    return version_;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    assert(key != "");
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}