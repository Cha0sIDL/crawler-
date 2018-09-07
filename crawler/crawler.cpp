#include <iostream>
#include <curl/curl.h>
#include <string>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>

using namespace std;

//libcurl接受响应的时候进行调用，将libcurl获取的html界面存储到自己的存储缓冲区userdata中
size_t WriteDate(char* ptr,size_t size,size_t mem,void* userdata)
{
    std::string* output=(std::string*)userdata;
    output->append(ptr,size*mem);
    return size*mem;
}
class ScopedHandler
{
    //运用智能指针的思想，在析构函数中关闭handler句柄这样在程序每次执行结束调用析构函数的时候就会自动关闭handler句柄
public:
    ScopedHandler(CURL* h):handler(h){}
    ~ScopedHandler(){curl_easy_cleanup(handler);}
private:
    CURL* handler;
};
/////////////////////////////////////////////////
//使用libcurl库通过url抓取网页html界面
/////////////////////////////////////////////////
/////////////////////////////////////////////////
bool OpenPage(const std::string& url,std::string *html)
{
    //初始化句柄
    CURL* handler=curl_easy_init();
    ScopedHandler scoped_handler(handler);
    //将句柄传入，第二个参数为传入指向url的指针，并将url转化为C风格字符串
    curl_easy_setopt(handler,CURLOPT_URL,url.c_str());
    //将数据保存
    curl_easy_setopt(handler,CURLOPT_WRITEFUNCTION,WriteDate);
    curl_easy_setopt(handler,CURLOPT_WRITEDATA,html);
    //发送请求
    CURLcode ret=curl_easy_perform(handler);
    if(ret!=CURLE_OK)
    {
        fprintf(stderr,"error\n");
        return false;
    }
    return true;
}
void test1()
{
    std::string html;
    OpenPage("http://www.shengxu6.com/book/1.html",&html);
        printf("%s\n",html.c_str());
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////
//解析主页章节的url
//使用正则表达式
/////////////////////////////////////////////////
/////////////////////////////////////////////////
void ParseMainPage(const std::string &html,std::vector<string>*url_list)
{
    boost::regex reg("/read/\S+html");
    //使用迭代器对字符串查找
    std::string::const_iterator cur=html.begin();
    std::string::const_iterator end=html.end();
    boost::smatch result;
    while(boost::regex_search(cur,end,result,reg))
    {
        url_list->push_back("http://www.shengxu6.com"+result[0]);
        //每次从下一个字符开始查找
        
        cur=result[0].second;
    }
}
void test2()
{
    std::string html;
    OpenPage("http://www.shengxu6.com/book/1.html",&html);
    std::vector<std::string> url_list;
    ParseMainPage(html,&url_list);
    for(size_t i=0;i<url_list.size();++i)
    {
        cout<<url_list[i].c_str()<<endl;
    }
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////
//获取抓到的url的界面和第一步一样直接调用函数即可
/////////////////////////////////////////////////
/////////////////////////////////////////////////
void test3()
{
    std::string html;
    OpenPage("http://www.shengxu6.com/read/1_1.html",&html);
    cout<<html.c_str()<<endl;
}
////////////////////////////////////////////////
////////////////////////////////////////////////
//从获取的html界面中过滤正文
////////////////////////////////////////////////
////////////////////////////////////////////////
void Parse_filtration(const std::string& html,std::string* content)
{
    // 使用字符串查找的方式查找出正文
    // 若使用正则表达式会略微麻烦
    // 先找到正文的开始和结束，然后去掉开始结束标记即可
    std::string begin_flag="<div class=\"panel-body content-body content-ext\">";
        size_t begin=html.find(begin_flag);
    //若返回npos则没找到
    if(begin==std::string::npos)
    {
        fprintf(stderr,"没有找到开始标记\n");
        return;
    }
    begin+=begin_flag.size();
    std::string end_flag="<script>_drgd200();</script>";
    size_t end=html.find(end_flag);
    if(end==std::string::npos)
    {
        fprintf(stderr,"没有找到结束标记\n");
        return;
    }
    if(begin>=end)
    {
        fprintf(stderr,"查找失败\n");
        return;
    }
    *content=html.substr(begin,end-begin);
    //替换转义字符
    boost::algorithm::replace_all(*content,"&nbsp;"," ");
    boost::algorithm::replace_all(*content,"<br />","\n");
    return;
}
void test4()
{
    std::string html;
      OpenPage("http://www.shengxu6.com/read/1_1.html",&html);
        std::string content;
          Parse_filtration(html, &content);
            printf("%s\n", content.c_str());
}
void RunAll()
{//打开主页
    std::string html;
    OpenPage("http://www.shengxu6.com/book/1.html",&html);
//从HTML界面中过滤掉无用的信息只留下小说链接
    std::vector<std::string> url_list;
    ParseMainPage(html,&url_list);
//打开小说链接
    for(size_t i=0;i<url_list.size();++i)
    {//循环遍历，解析每个章节的正文内容
        fprintf(stderr,"%s\n",url_list[i].c_str());
        OpenPage(url_list[i],&html);
        std::string content;
        Parse_filtration(html,&content);
        cout<<content.c_str()<<endl;
    }
}
int main()
{
    test4();
    //RunAll();
    return 0;

}

