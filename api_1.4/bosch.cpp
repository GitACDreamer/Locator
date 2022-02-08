﻿#include "bosch.h"
std::string hostId;

CURLcode HttpClient::HttpPost(const std::string &strUrl, std::string szJson, std::string &strResponse, int nTimeout)
{
  CURLcode res;
  char szJsonData[10240]; // for licensingFeatureSet change from 1024 to 10240
  memset(szJsonData, 0, sizeof(szJsonData));
  strcpy(szJsonData, szJson.c_str());
  CURL *pCURL = curl_easy_init();
  struct curl_slist *headers = NULL;
  if (pCURL == NULL)
  {
    return CURLE_FAILED_INIT;
  }

  CURLcode ret;
  ret = curl_easy_setopt(pCURL, CURLOPT_URL, strUrl.c_str());
  //    std::cout << ret << std::endl;

  ret = curl_easy_setopt(pCURL, CURLOPT_POST, 1L);
  headers = curl_slist_append(headers, "Expect:");
  headers = curl_slist_append(headers, "content-type:application/json");
  ret = curl_easy_setopt(pCURL, CURLOPT_HTTPHEADER, headers);
  ret = curl_easy_setopt(pCURL, CURLOPT_POSTFIELDS, szJsonData);
  //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  ret = curl_easy_setopt(pCURL, CURLOPT_TIMEOUT, nTimeout);
  ret = curl_easy_setopt(pCURL, CURLOPT_WRITEFUNCTION, HttpClient::receive_data);
  ret = curl_easy_setopt(pCURL, CURLOPT_WRITEDATA, (void *)&strResponse);
  res = curl_easy_perform(pCURL);
  curl_easy_cleanup(pCURL);
  return res;
}

size_t HttpClient::receive_data(void *contents, size_t size, size_t nmemb, void *stream)
{
  std::string *str = (std::string *)stream;
  (*str).append((char *)contents, size * nmemb);
  return size * nmemb;
}

Bosch::Bosch(std::string host_name, std::string username, std::string password)
{
  host_name_ = host_name;
  username_ = username;
  password_ = password;
}

// base 64 encode
const char *base64_chars[2] = {
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "+/",

    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "-_"};

// base 64 encode
std::string base64_encode(unsigned char const *bytes_to_encode, size_t in_len, bool url)
{

  size_t len_encoded = (in_len + 2) / 3 * 4;

  unsigned char trailing_char = url ? '.' : '=';

  //
  // Choose set of base64 characters. They differ
  // for the last two positions, depending on the url
  // parameter.
  // A bool (as is the parameter url) is guaranteed
  // to evaluate to either 0 or 1 in C++ therfore,
  // the correct character set is chosen by subscripting
  // base64_chars with url.
  //
  const char *base64_chars_ = base64_chars[url];

  std::string ret;
  ret.reserve(len_encoded);

  unsigned int pos = 0;

  while (pos < in_len)
  {
    ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0xfc) >> 2]);

    if (pos + 1 < in_len)
    {
      ret.push_back(base64_chars_[((bytes_to_encode[pos + 0] & 0x03) << 4) + ((bytes_to_encode[pos + 1] & 0xf0) >> 4)]);

      if (pos + 2 < in_len)
      {
        ret.push_back(base64_chars_[((bytes_to_encode[pos + 1] & 0x0f) << 2) + ((bytes_to_encode[pos + 2] & 0xc0) >> 6)]);
        ret.push_back(base64_chars_[bytes_to_encode[pos + 2] & 0x3f]);
      }
      else
      {
        ret.push_back(base64_chars_[(bytes_to_encode[pos + 1] & 0x0f) << 2]);
        ret.push_back(trailing_char);
      }
    }
    else
    {

      ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0x03) << 4]);
      ret.push_back(trailing_char);
      ret.push_back(trailing_char);
    }

    pos += 3;
  }

  return ret;
}

// base 64 decode
static const std::string base64_chars_decode =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// base 64 encode
static inline bool is_base64(const char c)
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

// base 64 encode
std::string base64_decode(std::string const &encoded_string)
{
  int in_len = (int)encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
  {
    char_array_4[i++] = encoded_string[in_];
    in_++;
    if (i == 4)
    {
      for (i = 0; i < 4; i++)
        char_array_4[i] = base64_chars_decode.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }
  if (i)
  {
    for (j = i; j < 4; j++)
      char_array_4[j] = 0;

    for (j = 0; j < 4; j++)
      char_array_4[j] = base64_chars_decode.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++)
      ret += char_array_3[j];
  }

  return ret;
}

//8.1.1 aboutModulesList
bool Bosch::aboutModulesList()
{
  std::cout << "Do AboutModulesList.\n"
            << std::endl;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "aboutModulesList";
  request["params"]["query"] = Json::object();

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<<str_response<<std::endl;

  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    aboutModulesList Success. \n"
                << std::endl;
      auto modules = json_response["result"]["response"]["modules"];
      std::cout << "modules:" << modules << std::endl;
      return true;
    }
    else
    {
      std::cout << "    aboutModulesList Failed. \n"
                << std::endl;
      std::cout << "    aboutModulesList response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//8.1.2 aboutModulesComponent: queries the component type
bool Bosch::aboutModulesComponent()
{
    std::cout << "Do aboutModulesComponent. \n"
              << std::endl;

    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "aboutModulesComponent";
    request["params"]["query"] = Json::object();
    //request["params"]["query"]["sessionId"] = session_id_;

    // post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

    //get error or necessary info
    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        if (response_code == 0)
        {
            std::cout << "    aboutModulesComponent Success. \n"
                      << std::endl;
            auto modulesComponent = json_response["result"]["response"]["component"];
            std::cout << "modulesComponent:" << modulesComponent << std::endl;
            return true;
        }
        else if (response_code == 11)
        {

            std::cout << "    aboutModulesComponent Failed (Code == 11). \n"
                      << std::endl;
            return false;
        }
        else
        {
            std::cout << "    aboutModulesComponent Failed. \n"
                      << str_response << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

// 8.2.1 sessionLogin
bool Bosch::sessionLogin(int port)
{
  std::cout << "Do sessionLogin. \n"
            << std::endl;
  //std::cout<<"    Different Module has different Session_ID";
  std::string port_str = std::to_string(port);
  //check if port is correct
  switch (port)
  {
  case 8080:
    std::cout << "   Port is 8080 and can operate Chapter 8 Shared RPC Method & Chapter 9 JSON RPC Localization Client Support "
              << "\n"
              << std::endl;
    break;
  case 8082:
    std::cout << "   Port is 8082 and can operate Chapter 8 Shared RPC Method & Chapter 10 JSON RPC Map Server "
              << "\n"
              << std::endl;
    break;
  case 8084:
    std::cout << "   Port is 8084 and can operate Chapter 11 JSON RPC Localization Client Support "
              << "\n"
              << std::endl;
    break;
  case 8086:
    std::cout << "   Port is 8086 and can operate Chapter 12 JSON RPC Map Server "
              << "\n"
              << std::endl;
    break;
  }
  //set json info
  Json timeout;
  timeout["valid"] = false;
  timeout["time"] = 86400;
  timeout["resolution"] = 1;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "sessionLogin";
  request["params"]["query"]["timeout"] = timeout;
  request["params"]["query"]["userName"] = username_;
  request["params"]["query"]["password"] = password_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":" + port_str;
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  std::cout << "session login response: \n" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      session_id_ = json_response["result"]["response"]["sessionId"];
      std::cout << "    sessionLogin Success. \n"
                << std::endl;
      std::cout << "    SessionId is " << session_id_ << "\n"
                << std::endl;
      return true;
    }
    else
    {
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//8.2.2 sessionLogout
bool Bosch::sessionLogout()
{
  std::cout << "Do Session Logout.\n"
            << std::endl;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "sessionLogout";
  request["params"]["query"]["sessionId"] = session_id_;
  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<< str_response<<std::endl;

  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    sessionLogout Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    sessionLogout Failed. \n"
                << std::endl;
      std::cout << "    sessionLogout response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//8.2.3 sessionRefresh
bool Bosch::sessionRefresh()
{
  std::cout << "Do Session Refresh.\n"
            << std::endl;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "sessionRefresh";
  request["params"]["query"]["sessionId"] = session_id_;
  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  std::cout << str_response << std::endl;

  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    sessionRefresh Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    sessionRefresh Failed. \n"
                << std::endl;
      std::cout << "    sessionRefresh response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//8.2.4 sessionGroupInfo
bool Bosch::sessionGroupInfo()
{
  std::cout << "Do sessionGroupInfo.\n"
            << std::endl;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "sessionGroupInfo";
  request["params"]["query"]["sessionId"] = session_id_;
  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<< str_response<<std::endl;

  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    sessionGroupInfo Success. \n"
                << std::endl;
      auto key_value = json_response["result"]["response"]["userGroups"];
      std::cout << "userGroups:" << key_value << std::endl;
      return true;
    }
    else
    {
      std::cout << "    sessionGroupInfo Failed. \n"
                << std::endl;
      std::cout << "    sessionGroupInfo response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//8.2.5 sessionList: returns a list of current session info objects
bool Bosch::sessionList()
{
    std::cout << "Do sessionList.\n"
              << std::endl;
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "sessionList";
    request["params"]["query"]["sessionId"] = session_id_;
    //post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
    //std::cout<< str_response<<std::endl;

    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        if (response_code == 0)
        {
            auto sessionList = json_response["result"]["response"]["sessions"];
            //std::cout << "sessionList: " << sessionList << std::endl;
            for (int i = 0; i < sessionList.size(); i = i + 1)
            {
                std::cout << "    " << sessionList[i] << std::endl;
            }
            std::cout << "    sessionList Success. \n"
                      << std::endl;
            return true;
        }
        else
        {
            std::cout << "    sessionList Failed. \n"
                      << std::endl;
            std::cout << "    sessionList response: \n"
                      << str_response << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

//8.2.6 sessionRevoke:  revokes a session with a revokation id from an session info object
bool Bosch::sessionRevoke(std::string revokeId)
{
    std::cout << "Do sessionRevoke.\n"
              << std::endl;
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "sessionRevoke";
    request["params"]["query"]["sessionId"] = session_id_;
    request["params"]["query"]["revokeId"] = revokeId;
    //post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
    //std::cout<< str_response<<std::endl;

    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        if (response_code == 0)
        {
            std::cout << "    sessionRevoke Success. \n"
                      << std::endl;
            return true;
        }
        else
        {
            std::cout << "    sessionRevoke Failed. \n"
                      << std::endl;
            std::cout << "    sessionRevoke response: \n"
                      << str_response << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

//8.2.7 sessionRevokeAll: revokes all sessions including the one usedto call this method
bool Bosch::sessionRevokeAll()
{
    std::cout << "Do sessionRevokeAll.\n"
              << std::endl;
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "sessionRevokeAll";
    request["params"]["query"]["sessionId"] = session_id_;
    //post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
    //std::cout<< str_response<<std::endl;

    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        if (response_code == 0)
        {
            std::cout << "    sessionRevokeAll Success. \n"
                      << std::endl;
            return true;
        }
        else
        {
            std::cout << "    sessionRevokeAll Failed. \n"
                      << std::endl;
            std::cout << "    sessionRevokeAll response: \n"
                      << str_response << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

//8.2.8 sessionRevokeId: returns the revokation id of the calling session
bool Bosch::sessionRevokeId()
{
    std::cout << "Do sessionRevokeId.\n"
              << std::endl;
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "sessionRevokeId";
    request["params"]["query"]["sessionId"] = session_id_;
    //post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
    //std::cout<< str_response<<std::endl;

    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        if (response_code == 0)
        {
            auto sessionRevokeId = json_response["result"]["response"]["revokeId"];
            std::cout << "sessionRevokeId: " << sessionRevokeId << std::endl;
//            for (int i = 0; i < sessionRevokeId.size(); i = i + 1)
//            {
//                std::cout << "    " << sessionRevokeId[i] << std::endl;
//            }
            std::cout << "    sessionRevokeId Success. \n"
                      << std::endl;
            return true;
        }
        else
        {
            std::cout << "    sessionRevokeId Failed. \n"
                      << std::endl;
            std::cout << "    sessionRevokeId response: \n"
                      << str_response << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

// 8.3.1 diagnosticList
bool Bosch::diagnosticList()
{
  std::cout << "Do diagnosticList \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "diagnosticList";
  request["params"]["query"]["sessionId"] = session_id_;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<< str_response<<std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      auto key_value = json_response["result"]["response"]["diagnosticEntries"];
      for (int i = 0; i < key_value.size(); i = i + 1)
      {
        double unix_timestamps = key_value[i]["timestamp"]["time"].get<double>();
        time_t diagnos_info_time = unix_timestamps / 1000;
        std::cout << "    Time（UTC/GMT+08:00）" << ctime(&diagnos_info_time) << std::endl;
        std::cout << "    " << key_value[i] << std::endl;
        std::cout << "\n"
                  << std::endl;
      }
      std::cout << "    diagnosticList Success. \n"
                << std::endl;

      return true;
    }
    else
    {
      std::cout << "    diagnosticList Failed." << std::endl;
      std::cout << "    diagnosticList response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 8.3.2 diagnosticClear
bool Bosch::diagnosticClear()
{
  std::cout << "Do diagnosticClear \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "diagnosticClear";
  request["params"]["query"]["sessionId"] = session_id_;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    diagnosticClear Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    diagnosticClear Failed." << std::endl;
      std::cout << "    diagnosticClear response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 8.4.1 licensingFeatureGetTrustedPlatformModuleInformation
bool Bosch::licensingTPM()
{
  std::cout << "Do licensingTPM \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "licensingFeatureGetTrustedPlatformModuleInformation";
  request["params"]["query"]["sessionId"] = session_id_;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<<"response:"<<str_response<<std::endl ;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    long response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 844424930263041)
    {
      auto response = json_response["result"]["response"];
      std::cout << "response:" << response << std::endl;
      std::cout << "\n    licensingTPM Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    licensingTPM Failed." << std::endl;
      std::cout << "    licensingTPM response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 8.4.2 licensingFeatureGetHostId
bool Bosch::licensingFeatureGetHostId()
{
  std::cout << "Do licensingFeatureGetHostId \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "licensingFeatureGetHostId";
  request["params"]["query"]["sessionId"] = session_id_;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;

  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      hostId = json_response["result"]["response"]["hostId"];
      std::cout << "    licensingFeatureGetHostId Success. \n"
                << std::endl;
      std::cout << "    hostId is " << hostId << "\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    licensingFeatureGetHostId Failed." << std::endl;
      std::cout << "    licensingFeatureGetHostId response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 8.4.3 licensingFeatureSet
bool Bosch::licensingFeatureSet(std::string binfilename)
{
  std::cout << "Do licensingFeatureSet. \n"
            << std::endl;
  std::cout << "    If this programm stuck here, check if the name of binfile is correct and in the same folder ." << std::endl;
  //change bin file to base64 code
  std::ifstream is(binfilename, std::ifstream::binary);
  // get length of file:
  is.seekg(0, is.end);
  unsigned int length = is.tellg();
  is.seekg(0, is.beg);
  char *buffer = new char[length];
  is.read(buffer, length);
  const std::string myString(buffer, length);
  std::string binfileencode = base64_encode(reinterpret_cast<const unsigned char *>(myString.c_str()), myString.length(), false);
  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "licensingFeatureSet";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["licenseKey"]["contentEncoding"] = "base64";
  request["params"]["query"]["licenseKey"]["contentMediaType"] = "application/LicenseFile";
  request["params"]["query"]["licenseKey"]["content"] = binfileencode;
  //std::cout<<request.dump()<<std::endl;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    licensingFeatureSet Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    licensingFeatureSet Failed." << std::endl;
      std::cout << "    licensingFeatureSet response: \n"
                << str_response << std::endl;

      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//8.4.4 licensingFeatureList
bool Bosch::licensingFeatureList()
{
  std::cout << "Do licensingFeatureList.\n"
            << std::endl;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "licensingFeatureList";
  request["params"]["query"]["sessionId"] = session_id_;
  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<< str_response<<std::endl;

  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    licensingFeatureList Success. \n"
                << std::endl;
      auto key_value = json_response["result"]["response"];
      std::cout << "FeatureList:" << key_value << std::endl;
      return true;
    }
    else
    {
      std::cout << "    licensingFeatureList Failed. \n"
                << std::endl;
      std::cout << "    licensingFeatureList response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 8.5.1 configList
bool Bosch::configList()
{
  std::cout << "Do configList. \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "configList";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  // print  response info
  //std::cout << "configlist response: \n" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);

    // output config data
    auto key_value = json_response["result"]["response"]["configEntries"];
    std::cout << "    Config info from Locator. (Related information on page 44/129 at APIDpcument 1.1 )\n"
              << std::endl;
    for (int i = 0; i < key_value.size(); i = i + 1)
    {
      std::cout << "    " << key_value[i] << std::endl;
    }
    //std::cout<<typeid(key_value).name()<<std::endl;
    //cout particular key or value if necessary
    /*
    std::cout<<key_value[1]["key"]<<std::endl;
    std::cout<<key_value[1]["value"]<<std::endl;
    std::cout<<typeid(key_value).name()<<std::endl;
    */

    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "\n    configlist Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    configList Failed." << std::endl;
      std::cout << "    configList response: \n"
                << str_response << std::endl;

      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//8.5.2 configDefaultList: queries a list of all configurable parameters with their default values
bool Bosch::configDefaultList()
{
    std::cout << "Do configDefaultList. \n"
              << std::endl;

    //set json info
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "configDefaultList";
    request["params"]["query"]["sessionId"] = session_id_;

    // post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

    //get error or necessary info
    Json json_response;
    try
    {
        json_response = Json::parse(str_response);

        // output config data
        auto key_value = json_response["result"]["response"]["configEntries"];
        std::cout << key_value << std::endl;
        std::cout << "    configDefaultList info from Locator. (Related information on page 44/129 at APIDpcument 1.1 )\n"
                  << std::endl;
        for (int i = 0; i < key_value.size(); i = i + 1)
        {
            std::cout << "    " << key_value[i] << std::endl;
        }

        int response_code = json_response["result"]["response"]["responseCode"];
        if (response_code == 0)
        {
            std::cout << "\n    configDefaultList Success. \n"
                      << std::endl;
            return true;
        }
        else
        {
            std::cout << "    configDefaultList Failed." << std::endl;
            std::cout << "    configDefaultList response: \n"
                      << str_response << std::endl;

            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

// 8.5.3 configSet string
bool Bosch::configSet_string(std::string key, std::string value, int port)
{
  //Config Param info from Locator. (Related information on page 44/129 at APIDpcument 1.1)
  std::cout << "Do configSet (Type == AsciiCharacterString || string || NetworkAddress ). \n"
            << std::endl;

  //set json info
  Json confparam_string;
  confparam_string["key"] = key;
  confparam_string["value"] = value;
  std::string port_str = std::to_string(port);

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "configSet";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["configEntries"] = Json::array({confparam_string});

  // post json info
  std::string str_url = "http://" + host_name_ + ":" + port_str;
  std::string str_response;
  std::string str_request;
  std::cout << request.dump() << std::endl;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  // print  response info
  std::cout << "configlist response: \n"
            << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    configSet_string Success. Now config method is" << confparam_string["key"] << ", config value is " << confparam_string["value"] << ".\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    configSet_string Failed. " << std::endl;
      std::cout << "    configSet_string response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 8.5.3 configSet number
bool Bosch::configSet_num(std::string key, double value, int port)
{
  //Config Param info from Locator. (Related information on page 44/129 at APIDpcument 1.1)
  std::cout << "Do configSet (Type == IEEE754Double || NonnegativeInteger64 ). \n"
            << std::endl;

  //set json info
  Json confparam_num;
  confparam_num["key"] = key;
  confparam_num["value"] = value;
  std::string port_str = std::to_string(port);

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "configSet";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["configEntries"] = Json::array({confparam_num});

  // post json info
  std::string str_url = "http://" + host_name_ + ":" + port_str;
  std::string str_response;
  std::string str_request;
  //std::cout<< request.dump()<<std::endl;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  // print  response info
  //std::cout << "configlist response: \n" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    configSet_num Success. Now config method is" << confparam_num["key"] << ", config value is " << confparam_num["value"] << ".\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    configSet_num Failed. " << std::endl;
      std::cout << "    configSet_num response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 8.5.3 configSet bool
bool Bosch::configSet_bool(std::string key, bool value, int port)
{
  //Config Param info from Locator. (Related information on page 44/129 at APIDpcument 1.1)
  std::cout << "Do configSet (Type == bool ). \n"
            << std::endl;

  //set json info
  Json confparam_bool;
  confparam_bool["key"] = key;
  confparam_bool["value"] = value;
  std::string port_str = std::to_string(port);

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "configSet";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["configEntries"] = Json::array({confparam_bool});

  // post json info
  std::string str_url = "http://" + host_name_ + ":" + port_str;
  std::string str_response;
  std::string str_request;
  std::cout << request.dump() << std::endl;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  // print  response info
  std::cout << "configlist response: \n"
            << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    configSet_bool Success. Now config method is" << confparam_bool["key"] << ", config value is " << confparam_bool["value"] << ".\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    configSet_bool (bool) Failed. " << std::endl;
      std::cout << "    configSet_bool response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 8.5.4 configSet array
bool Bosch::configSet_array(std::string key, std::vector<int> line_data)
{
  //Config Param info from Locator. (Related information on page 44/129 at APIDpcument 1.1)
  std::cout << "Do configSet (Type == arrya of IEEE754Double ). \n"
            << std::endl;

  //set json info
  auto range_line = Json::array();
  auto length = line_data.size();

  for (int i = 0; i < length; ++i)
  {
    int val = line_data[i];
    range_line.push_back(val);
  };

  Json confparam_array;
  confparam_array["key"] = key;
  confparam_array["value"] = range_line;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "configSet";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["configEntries"] = Json::array({confparam_array});

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  std::string str_request;
  //std::cout<< request.dump()<<std::endl;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  // print  response info
  //std::cout << "configlist response: \n" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    configSet_array Success. Now config method is" << confparam_array["key"] << ", config value is " << confparam_array["value"] << ".\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    configSet_array (array) Failed. " << std::endl;
      std::cout << "    configSet_array response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//8.6.1 aboutBuildList
bool Bosch::aboutBuildList()
{
  std::cout << "Do aboutBuildList.\n"
            << std::endl;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "aboutBuildList";
  request["params"]["query"]["sessionId"] = session_id_;
  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  std::cout << str_response << std::endl;

  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    aboutBuildList Success. \n"
                << std::endl;
      auto aboutString = json_response["result"]["response"]["aboutString"];
      std::cout << "aboutString:" << aboutString << std::endl;
      return true;
    }
    else
    {
      std::cout << "    aboutBuildList Failed. \n"
                << std::endl;
      std::cout << "    aboutBuildList response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//TODO 证书文件上传，待测试
//8.7.1 certificateSet
bool Bosch::certificateSet(std::string root, std::string crt, std::string key, std::string rev)
{
  std::cout << "Do certificateSet.\n"
            << std::endl;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "certificateSet";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["root"] = root;
  request["params"]["query"]["crt"] = crt;
  request["params"]["query"]["key"] = key;
  request["params"]["query"]["rev"] = rev;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  std::cout << str_response << std::endl;

  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    certificateSet Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    certificateSet Failed. \n"
                << std::endl;
      std::cout << "    certificateSet response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//8.8.1 systemShutdown
bool Bosch::systemShutdown()
{
  std::cout << "Do systemShutdown.\n"
            << std::endl;
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "systemShutdown";
  request["params"]["query"]["sessionId"] = session_id_;
  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  std::cout << str_response << std::endl;

  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    systemShutdown Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    systemShutdown Failed. \n"
                << std::endl;
      std::cout << "    systemShutdown response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.1.1 clientRecordingStart
bool Bosch::clientRecordingStart(std::string recordingName)
{
  std::cout << "Do clientRecordingStart. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientRecordingStart";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["recordingName"] = recordingName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientRecordingStart Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientRecordingStart Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first."
                << "\n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientRecordingStart Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << "\n"
                << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientRecordingStart Failed. \n"
                << std::endl;
      std::cout << "   clientRecordingStart response: \n"
                << "   " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.1.2 clientRecordingStop
bool Bosch::clientRecordingStop()
{
  std::cout << "Do clientRecordingStop. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientRecordingStop";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientRecordingStop Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 7)
    {
      std::cout << "    clientRecordingStop Failed (Code == 7). \n"
                << std::endl;
      std::cout << "    The current system state does not allow the operation to complete. Start Recording first. \n\n"
                << "     " << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "     clientRecordingStop Failed. \n"
                << std::endl;
      std::cout << "    clientRecordingStop response: \n"
                << "    " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.1.3 clientRecordingStartVisualRecording
bool Bosch::clientRecordingStartVisualRecording(std::string recordingName)
{
  std::cout << "Do clientRecordingStartVisualRecording. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientRecordingStartVisualRecording";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["recordingName"] = recordingName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientRecordingStartVisualRecording Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientRecordingStartVisualRecording Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientRecordingStartVisualRecording Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientRecordingStartVisualRecording Failed. \n"
                << std::endl;
      std::cout << "   clientRecordingStartVisualRecording response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.1.4 clientRecordingStopVisualRecording
bool Bosch::clientRecordingStopVisualRecording()
{
  std::cout << "Do clientRecordingStopVisualRecording. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientRecordingStopVisualRecording";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientRecordingStartVisualRecording Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 7)
    {
      std::cout << "    clientRecordingStopisualRecording Failed (Code == 7). \n"
                << std::endl;
      std::cout << "    The current system state does not allow the operation to complete. Start Recording first. \n"
                << "     " << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "     clientRecordingStopVisualRecording Failed. \n"
                << std::endl;
      std::cout << "    clientRecordingStopVisualRecording response: \n"
                << "    " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.15 clientRecordingList
bool Bosch::clientRecordingList()
{
  std::cout << "Do clientRecordingList. \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientRecordingList";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientRecordingList Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientRecordingList Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientRecordingList Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientRecordingList Failed. \n"
                << std::endl;
      std::cout << "   clientRecordingList response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.1.6 clientRecordingRename
bool Bosch::clientRecordingRename(std::string oldName, std::string newName)
{
  std::cout << "Do clientRecordingRename. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientRecordingRename";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["currentRecordingName"] = oldName;
  request["params"]["query"]["desiredRecordingName"] = newName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<<"response:"<<str_response<<std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientRecordingRename Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientRecordingRename Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientRecordingRename Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    clientRecordingRename Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientRecordingRename Failed. \n"
                << std::endl;
      std::cout << "   clientRecordingRename response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.1.7 clientRecordingDelete
bool Bosch::clientRecordingDelete(std::string recordingName)
{
  std::cout << "Do clientRecordingDelete. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientRecordingDelete";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["recordingName"] = recordingName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<<"response:"<<str_response<<std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientRecordingDelete Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientRecordingDelete Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientRecordingDelete Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    clientRecordingDelete Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientRecordingDelete Failed. \n"
                << std::endl;
      std::cout << "   clientRecordingDelete response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.1.8 clientRecordingSetCurrentPose
bool Bosch::clientRecordingSetCurrentPose(double x, double y, double angle)
{
  std::cout << "Do clientRecordingSetCurrentPose. \n"
            << std::endl;

  //set json info
  Pose2D pose;
  pose.x = x;
  pose.y = y;
  pose.a = angle;

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientRecordingSetCurrentPose";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["pose"]["x"] = pose.x;
  request["params"]["query"]["pose"]["y"] = pose.y;
  request["params"]["query"]["pose"]["a"] = pose.a;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<<"response:"<<str_response<<std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientRecordingSetCurrentPose Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientRecordingSetCurrentPose Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientRecordingSetCurrentPose Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    clientRecordingSetCurrentPose Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 7)
    {

      std::cout << "    clientRecordingSetCurrentPose Failed (Code == 7). \n"
                << std::endl;
      std::cout << "    The current system state does not allow the operation to complete. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientRecordingSetCurrentPose Failed. \n"
                << std::endl;
      std::cout << "   clientRecordingSetCurrentPose response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.2.1 clientMapStart: Requests to create a map from the data contained within a recording.(根据记录创建地图)
bool Bosch::clientMapStart(std::string recordingName, std::string clientMapName)
{
  std::cout << "Do clientMapStart. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientMapStart";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["recordingName"] = recordingName;
  request["params"]["query"]["clientMapName"] = clientMapName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientMapStart Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientMapStart Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientMapStart Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    clientMapStart Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientMapStart Failed. \n"
                << std::endl;
      std::cout << "   clientMapStart response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}
//9.2.2 clientMapStop: Requests to stop map creation.(停止创建地图)
bool Bosch::clientMapStop()
{
  std::cout << "Do clientMapStop. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientMapStop";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientMapStop Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientMapStop Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientMapStop Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    clientMapStop Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 7)
    {
      std::cout << "    clientMapStop Failed (Code == 7). \n"
                << std::endl;
      std::cout << "    The current system state does not allow the operation to complete. Start Map first. \n\n"
                << "     " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientMapStop Failed. \n"
                << std::endl;
      std::cout << "   clientMapStop response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}
//9.2.3 clientMapList: Retrieves all completed maps stored by the Localization Client.(检索定位客户端存储的所有完成的地图。)
bool Bosch::clientMapList()
{
  std::cout << "Do clientMapList. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientMapList";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientMapList Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientMapList Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientMapList Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    clientMapList Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientMapList Failed. \n"
                << std::endl;
      std::cout << "   clientMapList response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}
//9.2.4 clientMapRename: Renames a map stored by the Localization Client.(地图重命名)
bool Bosch::clientMapRename(std::string oldName, std::string newName)
{
  std::cout << "Do clientMapRename. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientMapRename";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["currentClientMapName"] = oldName;
  request["params"]["query"]["desiredClientMapName"] = newName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientMapRename Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientMapRename Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientMapRename Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    clientMapRename Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientMapRename Failed. \n"
                << std::endl;
      std::cout << "   clientMapRename response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}
//9.2.5 clientMapDelete: Delete a map stored by the Localization Client.(删除地图)
bool Bosch::clientMapDelete(std::string mapName)
{
  std::cout << "Do clientMapDelete. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientMapDelete";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["clientMapName"] = mapName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientMapDelete Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientMapDelete Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientMapDelete Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    clientMapDelete Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientMapDelete Failed. \n"
                << std::endl;
      std::cout << "   clientMapDelete response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}
//9.2.6 clientMapSend: Requests the Localization Client to send the map to the Map Server(发送地图到服务器)
bool Bosch::clientMapSend(std::string mapName)
{
  std::cout << "Do clientMapSend. \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientMapSend";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["clientMapName"] = mapName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientMapSend Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    clientMapSend Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    clientMapSend Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {
      std::cout << "    clientMapSend Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    clientMapSend Failed. \n"
                << std::endl;
      std::cout << "   clientMapSend response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.3.1 &9.3.2 clientLocalizationStart & Stop
bool Bosch::clientLocalizationSwitch(bool isStart)
{
  std::cout << "Do clientLocalizationSwitch. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = isStart ? "clientLocalizationStart" : "clientLocalizationStop";
  request["params"]["query"]["sessionId"] = session_id_;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "switch localization response: " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientLocalizationSwitch Success. Now " << request["method"] << "\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientLocalizationSwitch Failed. " << std::endl;
      std::cout << "    post clientLocalizationSwitch method is " << request["method"] << std::endl;
      std::cout << "    clientLocalizationSwitch: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.3.3 clientLocalizationSetSeed
bool Bosch::clientSetSeed(double x, double y, double a)
{
  std::cout << "Do clientLocalizationSetSeed. \n"
            << std::endl;

  //first define Pose2D
  Pose2D pose;
  pose.x = x;
  pose.y = y;
  pose.a = a;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientLocalizationSetSeed";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["enforceSeed"] = true;
  request["params"]["query"]["seedPose"]["x"] = pose.x;
  request["params"]["query"]["seedPose"]["y"] = pose.y;
  request["params"]["query"]["seedPose"]["a"] = pose.a;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "SetSeed response: " << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientLocalizationSetSeed Success.\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientLocalizationSetSeed Failed. " << std::endl;
      std::cout << "    clientLocalizationSetSeed response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.4.1 clientManualAlignStart
bool Bosch::clientManualAlignStart()
{
  std::cout << "Do clientManualAlignStart \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientManualAlignStart";
  request["params"]["query"]["sessionId"] = session_id_;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientManualAlignStart Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientManualAlignStart Failed." << std::endl;
      std::cout << "    clientManualAlignStart response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.4.2 clientManualAlignStop
bool Bosch::clientManualAlignStop()
{
  std::cout << "Do clientManualAlignStop \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientManualAlignStop";
  request["params"]["query"]["sessionId"] = session_id_;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {

      std::cout << "    clientManualAlignStop Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientManualAlignStop Failed." << std::endl;
      std::cout << "    clientManualAlignStop response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.4.3 clientManualAlignGetPointCloud
bool Bosch::clientManualAlignGetPointCloud(std::string clientMapName, int level)
{
  std::cout << "Do clientManualAlignGetPointCloud \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientManualAlignGetPointCloud";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["clientMapName"] = clientMapName;
  request["params"]["query"]["mapLevel"] = level;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {

      std::cout << "    clientManualAlignGetPointCloud Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 11)
    {
      auto pointCloud = json_response["result"]["response"]["pointCloud"];
      std::cout << "    clientManualAlignGetPointCloud Failed." << std::endl;
      std::cout << "    clientManualAlignGetPointCloud response: \n"
                << pointCloud << std::endl;
      return false;
    }
    else
    {
      std::cout << "    clientManualAlignGetPointCloud Failed." << std::endl;
      std::cout << "    clientManualAlignGetPointCloud response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.4.4 clientManualAlignSet
bool Bosch::clientManualAlignSet(double x, double y, double a, std::string mapName)
{
  Pose2D pose;
  pose.x = x;
  pose.y = y;
  pose.a = a;

  std::cout << "Do clientManualAlignSet \n"
            << std::endl;
  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientManualAlignSet";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["NEWalignOLD"]["x"] = pose.x;
  request["params"]["query"]["NEWalignOLD"]["y"] = pose.y;
  request["params"]["query"]["NEWalignOLD"]["a"] = pose.a;
  request["params"]["query"]["clientMapName"] = mapName;
  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {

      std::cout << "    clientManualAlignSet Success. \n"
                << std::endl;

      return true;
    }
    else if (response_code == 12)
    {
      std::cout << "   clientManualAlignSet Failed! (response_code ==12)" << std::endl;
      std::cout << "    clientManualAlignSet response: \n"
                << str_response << std::endl;
      std::cout << "    incorrect mape name . \n"
                << std::endl;
      return false;
    }
    else if (response_code == 7)
    {
      std::cout << "   clientManualAlignSet Failed! (response_code ==7)" << std::endl;
      std::cout << "    clientManualAlignSet response: \n"
                << str_response << std::endl;
      std::cout << "    first operate clientManualAlignStart. \n"
                << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "   clientManualAlignSet Failed! (response_code ==10)" << std::endl;
      std::cout << "    clientManualAlignSet response: \n"
                << str_response << std::endl;
      std::cout << "    Align already setted. \n"
                << std::endl;
      return false;
    }
    else
    {
      std::cout << "    clientManualAlignSet Failed." << std::endl;
      std::cout << "    clientManualAlignSet response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.5.1 clientGlobalAlignAddObservation
bool Bosch::clientGlobalAlignAddObservation(ClientGlobalAlignObservation observation)
{
  std::cout << "Do clientGlobalAlignAddObservation \n"
            << std::endl;
  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientGlobalAlignAddObservation";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["observation"]["landmarkName"] = observation.landmarkName;
  request["params"]["query"]["observation"]["sensorName"] = observation.sensorName;
  request["params"]["query"]["observation"]["LASERstaticCalibSENSOR"]["x"] = observation.laserStaticCalibSensor.x;
  request["params"]["query"]["observation"]["LASERstaticCalibSENSOR"]["y"] = observation.laserStaticCalibSensor.y;
  request["params"]["query"]["observation"]["LASERstaticCalibSENSOR"]["a"] = observation.laserStaticCalibSensor.a;
  request["params"]["query"]["observation"]["SENSORlandmark"]["x"] = observation.sensorLandmark.x;
  request["params"]["query"]["observation"]["SENSORlandmark"]["y"] = observation.sensorLandmark.y;
  request["params"]["query"]["observation"]["SENSORlandmark"]["a"] = observation.sensorLandmark.a;
  request["params"]["query"]["observation"]["MAPlandmark"]["x"] = observation.mapLandmark.x;
  request["params"]["query"]["observation"]["MAPlandmark"]["y"] = observation.mapLandmark.y;
  request["params"]["query"]["observation"]["MAPlandmark"]["a"] = observation.mapLandmark.a;
  request["params"]["query"]["observation"]["observationType"] = observation.obType;
  request["params"]["query"]["observation"]["calibrationType"] = observation.caliType;
  request["params"]["query"]["observation"]["hasOrientation"] = observation.hasOrientation;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "request:"<<request.dump()<<"response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {

      std::cout << "    clientGlobalAlignAddObservation Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 12)
    {
      std::cout << "   clientGlobalAlignAddObservation Failed! (response_code ==12)" << std::endl;
      std::cout << "    clientGlobalAlignAddObservation response: \n"
                << str_response << std::endl;
      return false;
    }
    else if (response_code == 7)
    {
      std::cout << "   clientGlobalAlignAddObservation Failed! (response_code ==7)" << std::endl;
      std::cout << "    clientGlobalAlignAddObservation response: \n"
                << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "   clientGlobalAlignAddObservation Failed! (response_code ==10)" << std::endl;
      std::cout << "    clientGlobalAlignAddObservation response: \n"
                << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    clientGlobalAlignAddObservation Failed." << std::endl;
      std::cout << "    clientGlobalAlignAddObservation response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.5.2 clientGlobalAlignReplaceObservations
bool Bosch::clientGlobalAlignReplaceObservations(std::string recordingName, std::vector<ClientGlobalAlignObservation> observation)
{
  std::cout << "Do clientGlobalAlignReplaceObservations\n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientGlobalAlignReplaceObservations";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["recordingName"] = recordingName;
  //request["params"]["query"]["replacementObservations"] = Json::parse(observation) ;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<<"request:"<<request.dump()<<std::endl<<"response:"<<str_response<<std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {

      std::cout << "    clientGlobalAlignReplaceObservations Success. \n"
                << std::endl;

      return true;
    }
    else if (response_code == 12)
    {
      std::cout << "   clientGlobalAlignReplaceObservations Failed! (response_code ==12)" << std::endl;
      std::cout << "    clientGlobalAlignReplaceObservations response: \n"
                << str_response << std::endl;
      return false;
    }
    else if (response_code == 7)
    {
      std::cout << "   clientGlobalAlignReplaceObservations Failed! (response_code ==7)" << std::endl;
      std::cout << "    clientGlobalAlignReplaceObservations response: \n"
                << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "   clientGlobalAlignReplaceObservations Failed! (response_code ==10)" << std::endl;
      std::cout << "    clientGlobalAlignReplaceObservations response: \n"
                << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    clientGlobalAlignReplaceObservations Failed." << std::endl;
      std::cout << "    clientGlobalAlignReplaceObservations response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.5.3 clientGlobalAlignDeleteObservations
bool Bosch::clientGlobalAlignDeleteObservations(std::string recordingName, std::vector<int> observationIds)
{
  std::cout << "Do clientGlobalAlignDeleteObservations \n"
            << std::endl;
  auto array = Json::array();
  for (unsigned int i = 0; i < observationIds.size(); ++i)
  {
    array.push_back(observationIds[i]);
  }
  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientGlobalAlignDeleteObservations";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["recordingName"] = recordingName;
  request["params"]["query"]["observationIds"] = array;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {

      std::cout << "    clientGlobalAlignDeleteObservations Success. \n"
                << std::endl;

      return true;
    }
    else if (response_code == 12)
    {
      std::cout << "   clientGlobalAlignDeleteObservations Failed! (response_code ==12)" << std::endl;
      std::cout << "    clientGlobalAlignDeleteObservations response: \n"
                << str_response << std::endl;
      return false;
    }
    else if (response_code == 7)
    {
      std::cout << "   clientGlobalAlignDeleteObservations Failed! (response_code ==7)" << std::endl;
      std::cout << "    clientGlobalAlignDeleteObservations response: \n"
                << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "   clientGlobalAlignDeleteObservations Failed! (response_code ==10)" << std::endl;
      std::cout << "    clientGlobalAlignDeleteObservations response: \n"
                << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    clientGlobalAlignDeleteObservations Failed." << std::endl;
      std::cout << "    clientGlobalAlignDeleteObservations response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.5.4 clientGlobalAlignListObservations
bool Bosch::clientGlobalAlignListObservations(std::string recordingName)
{
  std::cout << "Do clientGlobalAlignListObservations \n"
            << std::endl;
  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientGlobalAlignListObservations";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["recordingName"] = recordingName;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<<"response:"<<str_response<<std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {

      std::cout << "    clientGlobalAlignListObservations Success. \n"
                << std::endl;

      return true;
    }
    else if (response_code == 12)
    {
      std::cout << "   clientGlobalAlignListObservations Failed! (response_code ==12)" << std::endl;
      std::cout << "    clientGlobalAlignListObservations response: \n"
                << str_response << std::endl;
      return false;
    }
    else if (response_code == 7)
    {
      std::cout << "   clientGlobalAlignListObservations Failed! (response_code ==7)" << std::endl;
      std::cout << "    clientGlobalAlignListObservations response: \n"
                << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "   clientGlobalAlignListObservations Failed! (response_code ==10)" << std::endl;
      std::cout << "    clientGlobalAlignListObservations response: \n"
                << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    clientGlobalAlignListObservations Failed." << std::endl;
      std::cout << "    clientGlobalAlignListObservations response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.6.1 clientLaserMaskingStart
bool Bosch::clientLaserMaskingStart()
{
  std::cout << "Do clientLaserMaskingStart. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientLaserMaskingStart";
  request["params"]["query"]["sessionId"] = session_id_;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "clientLaserMaskingStart response: " << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientLaserMaskingStart Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 2)
    {
      std::cout << "    clientLaserMaskingStart Failed! " << std::endl;
      std::cout << "    clientLaserMaskingStart response: \n"
                << str_response << std::endl;
      std::cout << "    connect Lidar first, after recieve the laser scan then you can start MAKING. \n"
                << std::endl;
      return false;
    }
    else
    {
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.6.2 clientLaserMaskingStop
bool Bosch::clientLaserMaskingStop()
{
  std::cout << "Do clientLaserMaskingStop. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientLaserMaskingStop";
  request["params"]["query"]["sessionId"] = session_id_;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "clientLaserMaskingStart response: " << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientLaserMaskingStop Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientLaserMaskingStop Failed. " << std::endl;
      std::cout << "    clientLaserMaskingStop response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.6.3 clientLaserMaskingGetScan
bool Bosch::clientLaserMaskingGetScan()
{
  std::cout << "Do clientLaserMaskingGetScan. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientLaserMaskingGetScan";
  request["params"]["query"]["sessionId"] = session_id_;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "clientLaserMaskingGetScan response: " << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);

    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      //cout the inportant data
      std::cout << "    Some Important info about this Masking Scan" << std::endl;
      std::cout << "    maxRange is " << json_response["result"]["response"]["maxRange"] << std::endl;
      std::cout << "    numBeams is " << json_response["result"]["response"]["numBeams"] << std::endl;
      std::cout << "    Scan time is " << json_response["result"]["response"]["timeStart"]["time"] << "\n"
                << std::endl;
      std::cout << "    clientLaserMaskingGetScan Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 7)
    {
      std::cout << "    clientLaserMaskingGetScan Failed! \n " << std::endl;
      std::cout << "    clientLaserMaskingGetScan response: " << str_response << "\n"
                << std::endl;
      std::cout << "    Possible Reason: Lack of step 1:Connect Lidar or 2:start MAKING,after that operate step3: GetScan Masking \n"
                << std::endl;
      return false;
    }
    else
    {
      std::cout << "    clientLaserMaskingGetScan Failed! \n"
                << std::endl;
      std::cout << "    clientLaserMaskingGetScan response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.7.2 clientUserAdd
bool Bosch::clientUserAdd(std::string username, std::string password, std::string group)
{
  std::cout << "Do clientUserAdd. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientUserAdd";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["userName"] = username;
  request["params"]["query"]["userPassword"] = password;
  request["params"]["query"]["userGroups"] = Json::array({group});

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  std::cout << "response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientUserAdd Success.\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientUserAdd Failed! \n"
                << std::endl;
      std::cout << "    clientUserAdd response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.7.3 clientUserDelete
bool Bosch::clientUserDelete(std::string username)
{
  std::cout << "Do clientUserDelete. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientUserDelete";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["userName"] = username;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  std::cout << "response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientUserDelete Success.\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientUserDelete Failed! \n"
                << std::endl;
      std::cout << "    clientUserDelete response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.7.4 clientUserChangePassword
bool Bosch::clientUserChangePassword(std::string newpassword)
{
  std::cout << "Do clientUserChangePassword. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientUserChangePassword";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["userName"] = username_;
  request["params"]["query"]["newUserPassword"] = newpassword;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "changepassword response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientUserChangePassword Success.\n"
                << std::endl;
      std::cout << "   newUserPassword is " << request["params"]["query"]["newUserPassword"] << "\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientUserChangePassword Failed! \n"
                << std::endl;
      std::cout << "    clientUserChangePassword response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 9.7.5 clientUserChangeOwnPassword
bool Bosch::clientUserChangeOwnPassword(std::string oldPassword, std::string newPassword)
{
  std::cout << "Do clientUserChangeOwnPassword. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientUserChangeOwnPassword";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["newUserPassword"] = newPassword;
  request["params"]["query"]["oldUserPassword"] = oldPassword;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "changepassword response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientUserChangeOwnPassword Success.\n"
                << std::endl;
      std::cout << "   newUserPassword is " << request["params"]["query"]["newUserPassword"] << "\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientUserChangeOwnPassword Failed! \n"
                << std::endl;
      std::cout << "    clientUserChangeOwnPassword response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.7.6 clientUserList
bool Bosch::clientUserList()
{
  std::cout << "Do clientUserList. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientUserList";
  request["params"]["query"]["sessionId"] = session_id_;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientUserList Success.\n"
                << std::endl;
      auto userInfoList = json["result"]["response"]["userInfoList"];
      std::cout << "      userInfoList:\n"
                << userInfoList << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientUserList Failed! \n"
                << std::endl;
      std::cout << "    clientUserList response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.7.7 clientUserListGroup
bool Bosch::clientUserListGroup()
{
  std::cout << "Do clientUserListGroup. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "clientUserListGroup";
  request["params"]["query"]["sessionId"] = session_id_;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8080";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    clientUserListGroup Success.\n"
                << std::endl;
      auto userGroups = json["result"]["response"]["userGroups"];
      std::cout << "      userGroups:\n"
                << userGroups << std::endl;
      return true;
    }
    else
    {
      std::cout << "    clientUserListGroup Failed! \n"
                << std::endl;
      std::cout << "    clientUserListGroup response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//9.8 Map Expansion (Module: ClientExpandMap)
//9.8.1 clientExpandMapEnable: Enables map expansion and requests to enter the control mode EXPANDMAP
bool Bosch::clientExpandMapEnable(std::string clientExpandMapPriorMapName)
{
    std::cout << "Do clientExpandMapEnable. \n"
              << std::endl;

    //set json info
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "clientExpandMapEnable";
    request["params"]["query"]["sessionId"] = session_id_;
    request["params"]["query"]["priorMapName"] = clientExpandMapPriorMapName;

    // post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
    //std::cout << "response:" << str_response << std::endl;

    //get error or necessary info
    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        std::cout << response_code << std::endl;
        if (response_code == 0)
        {
            std::cout << "    clientExpandMapEnable Success. \n"
                      << std::endl;
            return true;
        }
        else
        {

            std::cout << "    clientExpandMapEnable Failed. \n"
                      << std::endl;
            std::cout << "   clientExpandMapEnable response: \n\n"
                      << "   " << str_response << "\n"
                      << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

//9.8.2 clientExpandMapDisable: Disables map expansion and requests to leave the control mode EXPANDMAP
bool Bosch::clientExpandMapDisable()
{
    std::cout << "Do clientExpandMapDisable. \n"
              << std::endl;

    //set json info
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "clientExpandMapDisable";
    request["params"]["query"]["sessionId"] = session_id_;

    // post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
    //std::cout << "response:" << str_response << std::endl;

    //get error or necessary info
    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        std::cout << response_code << std::endl;
        if (response_code == 0)
        {
            std::cout << "    clientExpandMapDisable Success. \n"
                      << std::endl;
            return true;
        }
        else
        {

            std::cout << "   clientExpandMapDisable Failed. \n"
                      << std::endl;
            std::cout << "   clientExpandMapDisable response: \n\n"
                      << "   " << str_response << "\n"
                      << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

////clientExpandMapAddOverwriteZone
//bool  Bosch::clientExpandMapAddOverwriteZone(std::string zoneName, int zoneType, std::vector<Position2D> polygonPoints)
//{
//    Zone zone;
//    zone.zoneName = zoneName;
//    zone.zoneType = zoneType;
//    zone.polygonPoints = polygonPoints;
//
//    //set json info
//    auto pointsX = Json::array();
//    auto lengthX = polygonPoints.size();
//
//    for (int i = 0; i < lengthX; ++i)
//    {
//        double val = zone.polygonPoints.at(i).x;
//        pointsX.push_back(val);
//    };
//
//    //set json info
//    auto pointsY = Json::array();
//    auto lengthY = polygonPoints.size();
//
//    for (int i = 0; i < lengthY; ++i)
//    {
//        double val = zone.polygonPoints.at(i).y;
//        pointsY.push_back(val);
//    };
//
//    Json confparam_array;
//    confparam_array["x"] = pointsX;
//    confparam_array["y"] = pointsY;
//
//    std::cout << "confparam: " << confparam_array["x"] << std::endl;
//
//    std::cout << "Do clientExpandMapAddOverwriteZone. \n"
//              << std::endl;
//
//    //set json info
//    Json request;
//    request["id"] = 1;
//    request["jsonrpc"] = "2.0";
//    request["method"] = "clientExpandMapAddOverwriteZone";
//    request["params"]["query"]["sessionId"] = session_id_;
////    request["params"]["query"]["zone"] = zone;
//    //request["params"]["query"]["zone"]["polygonPoints"] = zone.polygonPoints;
//    request["params"]["query"]["zone"]["zoneName"] = zone.zoneName;
//    request["params"]["query"]["zone"]["zoneType"] = zone.zoneType;
//    request["params"]["query"]["zone"]["polygonPoints"] = Json::array({confparam_array});
//
//    // post json info
//    std::string str_url = "http://" + host_name_ + ":8080";
//    std::string str_response;
//    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
//    std::cout << "response:" << str_response << std::endl;
//
//    //get error or necessary info
//    Json json_response;
//    try
//    {
//        json_response = Json::parse(str_response);
//        int response_code = json_response["result"]["response"]["responseCode"];
//        std::cout << response_code << std::endl;
//        if (response_code == 0)
//        {
//            std::cout << "    clientExpandMapAddOverwriteZone Success. \n"
//                      << std::endl;
//            return true;
//        }
//        else
//        {
//
//            std::cout << "   clientExpandMapAddOverwriteZone Failed. \n"
//                      << std::endl;
//            std::cout << "   clientExpandMapAddOverwriteZone response: \n\n"
//                      << "   " << str_response << "\n"
//                      << std::endl;
//            return false;
//        }
//    }
//    catch (const std::exception &e)
//    {
//        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
//        return false;
//    }
//}

//9.8.4 clientExpandMapDeleteOverwriteZones: Deletes overwrite zones in a saved recording
bool Bosch::clientExpandMapDeleteOverwriteZones(std::string recordingName, int zoneId)
{
    std::cout << "Do clientExpandMapDeleteOverwriteZones. \n"
              << std::endl;

    int array[1] = {zoneId};
    //set json info
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "clientExpandMapDeleteOverwriteZones";
    request["params"]["query"]["sessionId"] = session_id_;
    request["params"]["query"]["recordingName"] = recordingName;
    request["params"]["query"]["zoneIds"] = array;

    // post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
    //std::cout << "response:" << str_response << std::endl;

    //get error or necessary info
    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        std::cout << response_code << std::endl;
        if (response_code == 0)
        {
            std::cout << "    clientExpandMapDeleteOverwriteZones Success. \n"
                      << std::endl;
            return true;
        }
        else
        {

            std::cout << "   clientExpandMapDeleteOverwriteZones Failed. \n"
                      << std::endl;
            std::cout << "   clientExpandMapDeleteOverwriteZones response: \n\n"
                      << "   " << str_response << "\n"
                      << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

////clientExpandMapReplaceOverwriteZones
//bool Bosch::clientExpandMapReplaceOverwriteZones(std::string recordingName, int zoneId1, int zoneId2)
//{
//    std::cout << "Do clientExpandMapReplaceOverwriteZones. \n"
//              << std::endl;
//
//    int array[2] = {zoneId1,zoneId2};
//    //set json info
//    Json request;
//    request["id"] = 1;
//    request["jsonrpc"] = "2.0";
//    request["method"] = "clientExpandMapReplaceOverwriteZones";
//    request["params"]["query"]["sessionId"] = session_id_;
//    request["params"]["query"]["recordingName"] = recordingName;
//    request["params"]["query"]["replacementZones"] = array;
//
//    // post json info
//    std::string str_url = "http://" + host_name_ + ":8080";
//    std::string str_response;
//    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
//    //std::cout << "response:" << str_response << std::endl;
//
//    //get error or necessary info
//    Json json_response;
//    try
//    {
//        json_response = Json::parse(str_response);
//        int response_code = json_response["result"]["response"]["responseCode"];
//        std::cout << response_code << std::endl;
//        if (response_code == 0)
//        {
//            std::cout << "    clientExpandMapReplaceOverwriteZones Success. \n"
//                      << std::endl;
//            return true;
//        }
//        else
//        {
//
//            std::cout << "   clientExpandMapReplaceOverwriteZones Failed. \n"
//                      << std::endl;
//            std::cout << "   clientExpandMapReplaceOverwriteZones response: \n\n"
//                      << "   " << str_response << "\n"
//                      << std::endl;
//            return false;
//        }
//    }
//    catch (const std::exception &e)
//    {
//        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
//        return false;
//    }
//}

//9.8.6 clientExpandMapListOverwriteZones: Lists overwrite zones in an existing and finalized recording.
bool Bosch::clientExpandMapListOverwriteZones(std::string recordingName)
{
    std::cout << "Do clientExpandMapListOverwriteZones. \n"
              << std::endl;

    //set json info
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "clientExpandMapListOverwriteZones";
    request["params"]["query"]["sessionId"] = session_id_;
    request["params"]["query"]["recordingName"] = recordingName;

    // post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
    //std::cout << "response:" << str_response << std::endl;

    //get error or necessary info
    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        std::cout << response_code << std::endl;
        if (response_code == 0)
        {
            auto response = json_response["result"]["response"];
            std::cout << "response: " << response << std::endl;
            std::cout << "    clientExpandMapListOverwriteZones Success. \n"
                      << std::endl;
            return true;
        }
        else
        {

            std::cout << "   clientExpandMapListOverwriteZones Failed. \n"
                      << std::endl;
            std::cout << "   clientExpandMapListOverwriteZones response: \n\n"
                      << "   " << str_response << "\n"
                      << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

//9.8.7 clientExpandMapGetPriorMapPointCloud: Gets the prior map in an existing and finalized recording.
bool Bosch::clientExpandMapGetPriorMapPointCloud(std::string recordingName)
{
    std::cout << "Do clientExpandMapGetPriorMapPointCloud. \n"
              << std::endl;

    //set json info
    Json request;
    request["id"] = 1;
    request["jsonrpc"] = "2.0";
    request["method"] = "clientExpandMapGetPriorMapPointCloud";
    request["params"]["query"]["sessionId"] = session_id_;
    request["params"]["query"]["recordingName"] = recordingName;

    // post json info
    std::string str_url = "http://" + host_name_ + ":8080";
    std::string str_response;
    HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
    //std::cout << "response:" << str_response << std::endl;

    //get error or necessary info
    Json json_response;
    try
    {
        json_response = Json::parse(str_response);
        int response_code = json_response["result"]["response"]["responseCode"];
        std::cout << response_code << std::endl;
        if (response_code == 0)
        {
            auto response = json_response["result"]["response"];
            std::cout << "response: " << response << std::endl;
            std::cout << "    clientExpandMapGetPriorMapPointCloud Success. \n"
                      << std::endl;
            return true;
        }
        else
        {

            std::cout << "   clientExpandMapGetPriorMapPointCloud Failed. \n"
                      << std::endl;
            std::cout << "   clientExpandMapGetPriorMapPointCloud response: \n\n"
                      << "   " << str_response << "\n"
                      << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
        return false;
    }
}

// 10.1.1 serverMapGetInfo
bool Bosch::serverMapGetInfo(std::string mapName)
{
  std::cout << "Do serverMapGetInfo \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverMapGetInfo";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["serverMapName"] = mapName;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    //std::cout<<str_response<<std::endl;
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      //btw: "fingerprint", "type" and "level" (from response code) are not used at LLS release 1.1
      std::cout << "    nameofmap is " << mapName << "\n"
                << std::endl;
      if (json_response["result"]["response"]["loaded"] == true)
      {
        std::cout << "        the map is loaded into the Map Server’s memory \n"
                  << std::endl;
      }
      else
      {
        std::cout << "        the map is not loaded into the Map Server’s memory \n"
                  << std::endl;
      }

      if (json_response["result"]["response"]["createdAt"]["valid"] == true)
      {
        double unix_timestamps = json_response["result"]["response"]["createdAt"]["time"].get<double>();
        time_t diagnos_info_time = unix_timestamps / 1000000;
        std::cout << "        createdAt（UTC/GMT+08:00）:  " << ctime(&diagnos_info_time) << std::endl;
      }
      else if (json_response["result"]["response"]["createdAt"]["valid"] == false)
      {
        std::cout << "         map is not created, createdAt valid is false \n"
                  << std::endl;
      }

      if (json_response["result"]["response"]["updatedAt"]["valid"] == true)
      {
        double unix_timestamps = json_response["result"]["response"]["updatedAt"]["time"].get<double>();
        time_t diagnos_info_time = unix_timestamps / 1000000;
        std::cout << "        updatedAt（UTC/GMT+08:00）:  " << ctime(&diagnos_info_time) << std::endl;
      }
      else if (json_response["result"]["response"]["updatedAt"]["valid"] == false)
      {
        std::cout << "        map is not updated,updatedAt valid is false \n " << std::endl;
      }

      if (json_response["result"]["response"]["sentAt"]["valid"] == true)
      {
        double unix_timestamps = json_response["result"]["response"]["sentAt"]["time"].get<double>();
        time_t diagnos_info_time = unix_timestamps / 1000000;
        std::cout << "        sentAt（UTC/GMT+08:00）:   " << ctime(&diagnos_info_time) << std::endl;
      }
      else if (json_response["result"]["response"]["sentAt"]["valid"] == false)
      {
        std::cout << "        map is not sent, sentAt valid is false \n"
                  << std::endl;
      }

      if (json_response["result"]["response"]["receivedAt"]["valid"] == true)
      {
        double unix_timestamps = json_response["result"]["response"]["receivedAt"]["time"].get<double>();
        time_t diagnos_info_time = unix_timestamps / 1000000;
        std::cout << "        receivedAt（UTC/GMT+08:00）:  " << ctime(&diagnos_info_time) << std::endl;
      }
      else if (json_response["result"]["response"]["receivedAt"]["valid"] == false)
      {
        std::cout << "        map is not received, receivedAt valid is false \n"
                  << std::endl;
      }

      std::cout << "    serverMapGetInfo Success. \n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    serverMapGetInfo Failed." << std::endl;
      std::cout << "    serverMapGetInfo response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 10.1.2 serverMapList
bool Bosch::serverMapList()
{
  std::cout << "Do serverMapList \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverMapList";
  request["params"]["query"]["sessionId"] = session_id_;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    Map List from server"
                << "\n"
                << std::endl;
      auto key_value = json_response["result"]["response"]["serverMapNames"];
      for (int i = 0; i < key_value.size(); i = i + 1)
      {
        std::string mapName = key_value[i].get<std::string>();
        std::cout << "    " << mapName << std::endl;
      }
      std::cout << std::endl;
      std::cout << "    serverMapList Success. \n"
                << std::endl;

      return true;
    }
    else
    {
      std::cout << "    serverMapList Failed." << std::endl;
      std::cout << "    serverMapList response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 10.1.3 serverMapRename
bool Bosch::serverMapRename(std::string currentServerMapName, std::string desiredServerMapName)
{
  std::cout << "Do serverMapRename \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverMapRename";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["currentServerMapName"] = currentServerMapName;
  request["params"]["query"]["desiredServerMapName"] = desiredServerMapName;

  //post json info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverMapRename from server"
                << "\n"
                << std::endl;
      std::cout << "    serverMapList Success. \n"
                << std::endl;

      return true;
    }
    else if (response_code == 11)
    {

      std::cout << "    serverMapRename Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    serverMapRename Failed." << std::endl;
      std::cout << "    serverMapRename response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//10.1.4 serverMapDelete: Delete a map stored by the Map Server.(删除地图)
bool Bosch::serverMapDelete(std::string serverMapName)
{
  std::cout << "Do serverMapDelete. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverMapDelete";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["serverMapName"] = serverMapName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverMapDelete Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    serverMapDelete Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    serverMapDelete Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    serverMapDelete Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    serverMapDelete Failed. \n"
                << std::endl;
      std::cout << "   serverMapDelete response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//10.1.5 serverMapGetPointCloud
bool Bosch::serverMapGetPointCloud(std::string serverMapName)
{
  std::cout << "Do serverMapGetPointCloud. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverMapGetPointCloud";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["serverMapName"] = serverMapName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverMapGetPointCloud Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    serverMapGetPointCloud Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    serverMapGetPointCloud Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    serverMapGetPointCloud Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    serverMapGetPointCloud Failed. \n"
                << std::endl;
      std::cout << "   serverMapGetPointCloud response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//10.1.6 serverMapGetImage
bool Bosch::serverMapGetImage(int width, int height, int level, std::string serverMapName)
{
  std::cout << "Do serverMapGetImage. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverMapGetImage";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["serverMapName"] = serverMapName;
  request["params"]["query"]["width"] = width;
  request["params"]["query"]["height"] = height;
  request["params"]["query"]["level"] = level;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverMapGetImage Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    serverMapGetImage Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    serverMapGetImage Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    serverMapGetImage Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    serverMapGetImage Failed. \n"
                << std::endl;
      std::cout << "   serverMapGetImage response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//10.1.7 serverMapGetImageWithResolution
bool Bosch::serverMapGetImageWithResolution(std::string serverMapName, double resolution)
{
  std::cout << "Do serverMapGetImageWithResolution \n"
            << std::endl;

  //set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverMapGetImageWithResolution";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["serverMapName"] = serverMapName;
  request["params"]["query"]["resolution"] = resolution; // pixel/m

  //post json info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<<" send data is   "<<  request.dump()<< std::endl;
  //get error or necessary info
  //std::cout<< str_response<<std::endl;
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::string mapcode = "";
      mapcode = json_response["result"]["response"]["image"]["content"];
      std::string imgdecode64 = base64_decode(mapcode);
      std::fstream f2;
      std::string outputmapname = serverMapName + "_resolution_" + std::to_string(resolution);
      f2.open(outputmapname, std::ios::out | std::ios::binary);
      f2 << imgdecode64;
      f2.close();
      std::cout << "    serverMapGetImageWithResolution Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 9)
    {
      std::cout << "    serverMapGetImageWithResolution Failed! (response_code ==9)" << std::endl;
      std::cout << "    serverMapGetImageWithResolution response: \n"
                << str_response << std::endl;
      std::cout << "    improper resolution, unit is x pixel/m (try 20?) . \n"
                << std::endl;
      return false;
    }
    else if (response_code == 11)
    {
      std::cout << "    serverMapGetImageWithResolution Failed! (response_code ==12)" << std::endl;
      std::cout << "    serverMapGetImageWithResolution response: \n"
                << str_response << std::endl;
      std::cout << "    incorrect mape name . \n"
                << std::endl;
      return false;
    }
    else
    {
      std::cout << "    serverMapGetImageWithResolution Failed." << std::endl;
      std::cout << "    serverMapGetImageWithResolution response: \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//10.1.8 serverMapGetMapThumbnail
bool Bosch::serverMapGetMapThumbnail(std::string serverMapName, int level)
{
  std::cout << "Do serverMapGetMapThumbnail. \n"
            << std::endl;

  //set json info

  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverMapGetMapThumbnail";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["serverMapName"] = serverMapName;
  request["params"]["query"]["mapLevel"] = level;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response:" << str_response << std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverMapGetMapThumbnail Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 13)
    {
      std::cout << "    serverMapGetMapThumbnail Failed (Code == 13). \n"
                << std::endl;
      std::cout << "   A required sensor was not available. Connect Lidar first. \n\n"
                << "   " << str_response << std::endl;
      return false;
    }
    else if (response_code == 10)
    {
      std::cout << "    serverMapGetMapThumbnail Failed (Code == 10). \n"
                << std::endl;
      std::cout << "    Tried to create an entity that already exists. Change the name of recordingName. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else if (response_code == 11)
    {

      std::cout << "    serverMapGetMapThumbnail Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {

      std::cout << "    serverMapGetMapThumbnail Failed. \n"
                << std::endl;
      std::cout << "   serverMapGetMapThumbnail response: \n\n"
                << "   " << str_response << "\n"
                << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//10.2.2 serverUserAdd
bool Bosch::serverUserAdd(std::string username, std::string password, std::string group)
{
  std::cout << "Do serverUserAdd. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverUserAdd";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["userName"] = username;
  request["params"]["query"]["userPassword"] = password;
  request["params"]["query"]["userGroups"] = Json::array({group});

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  std::cout << "response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverUserAdd Success.\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    serverUserAdd Failed! \n"
                << std::endl;
      std::cout << "    serverUserAdd response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//10.2.3 serverUserDelete
bool Bosch::serverUserDelete(std::string username)
{
  std::cout << "Do serverUserDelete. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverUserDelete";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["userName"] = username;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  std::cout << "response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverUserDelete Success.\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    serverUserDelete Failed! \n"
                << std::endl;
      std::cout << "    serverUserDelete response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 10.2.4 serverUserChangePassword
bool Bosch::serverUserChangePassword(std::string newpassword)
{
  std::cout << "Do serverUserChangePassword. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverUserChangePassword";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["userName"] = username_;
  request["params"]["query"]["newUserPassword"] = newpassword;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "changepassword response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverUserChangePassword Success.\n"
                << std::endl;
      std::cout << "   newUserPassword is " << request["params"]["query"]["newUserPassword"] << "\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    serverUserChangePassword Failed! \n"
                << std::endl;
      std::cout << "    serverUserChangePassword response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

// 10.2.5 serverUserChangeOwnPassword
bool Bosch::serverUserChangeOwnPassword(std::string oldPassword, std::string newPassword)
{
  std::cout << "Do serverUserChangeOwnPassword. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverUserChangeOwnPassword";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["newUserPassword"] = newPassword;
  request["params"]["query"]["oldUserPassword"] = oldPassword;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "changepassword response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverUserChangeOwnPassword Success.\n"
                << std::endl;
      std::cout << "   newUserPassword is " << request["params"]["query"]["newUserPassword"] << "\n"
                << std::endl;
      return true;
    }
    else
    {
      std::cout << "    serverUserChangeOwnPassword Failed! \n"
                << std::endl;
      std::cout << "    serverUserChangeOwnPassword response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//10.2.6 serverUserList
bool Bosch::serverUserList()
{
  std::cout << "Do serverUserList. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverUserList";
  request["params"]["query"]["sessionId"] = session_id_;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverUserList Success.\n"
                << std::endl;
      auto userInfoList = json["result"]["response"]["userInfoList"];
      std::cout << "      userInfoList:\n"
                << userInfoList << std::endl;
      return true;
    }
    else
    {
      std::cout << "    serverUserList Failed! \n"
                << std::endl;
      std::cout << "    serverUserList response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//10.2.7 serverUserListGroup
bool Bosch::serverUserListGroup()
{
  std::cout << "Do serverUserListGroup. \n"
            << std::endl;

  // set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "serverUserListGroup";
  request["params"]["query"]["sessionId"] = session_id_;

  //post JSON info
  std::string str_url = "http://" + host_name_ + ":8082";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout << "response: \n " << str_response << std::endl;

  //get error or necessary info
  Json json;
  try
  {
    json = Json::parse(str_response);
    int response_code = json["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    serverUserListGroup Success.\n"
                << std::endl;
      auto userGroups = json["result"]["response"]["userGroups"];
      std::cout << "      userGroups:\n"
                << userGroups << std::endl;
      return true;
    }
    else
    {
      std::cout << "    serverUserListGroup Failed! \n"
                << std::endl;
      std::cout << "    serverUserListGroup response: " << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//11.1.1 supportReportCreate
bool Bosch::supportReportCreate()
{
  std::cout << "Do supportReportCreate. \n"
            << std::endl;
  std::string reportname = "";

  //supportReportCreate set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "supportReportCreate";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8084";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    supportReportCreate Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 11)
    {

      std::cout << "    supportReportCreate Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    supportReportCreate Failed. \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//11.1.2 supportReportCreateMinimal
bool Bosch::supportReportCreateMinimal()
{
  std::cout << "Do supportReportCreateMinimal. \n"
            << std::endl;
  std::string reportname = "";

  //supportReportCreateMinimal set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "supportReportCreateMinimal";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8084";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    supportReportCreateMinimal Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 11)
    {

      std::cout << "    supportReportCreateMinimal Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    supportReportCreateMinimal Failed. \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//11.1.3 supportReportSetDescription
bool Bosch::supportReportSetDescription(std::string reportDescription)
{
  std::cout << "Do supportReportSetDescription. \n"
            << std::endl;
  std::string reportname = "";

  //supportReportSetDescription set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "supportReportSetDescription";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["reportDescription"] = reportDescription;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8084";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    reportDescription Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 11)
    {

      std::cout << "    reportDescription Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    reportDescription Failed. \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//11.1.4 supportReportList
bool Bosch::supportReportList()
{
  std::cout << "Do supportReportList. \n"
            << std::endl;
  std::string reportname = "";

  //supportReportList set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "supportReportList";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8084";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);
  //std::cout<<"response:"<<str_response<<std::endl;

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    supportReportList Success. \n"
                << std::endl;
      auto reportNames = json_response["result"]["response"]["reportNames"];
      std::cout << reportNames << std::endl;

      return true;
    }
    else if (response_code == 11)
    {

      std::cout << "    supportReportList Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    supportReportList Failed. \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//11.1.1&11.1.5 supportReportCreate&supportReportCreateandgetpath
bool Bosch::supportReportCreateandgetpath()
{
  std::cout << "Do supportReportCreate. \n"
            << std::endl;
  std::string reportname = "";

  //supportReportCreate set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "supportReportCreate";
  request["params"]["query"]["sessionId"] = session_id_;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8084";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    supportReportCreate Success. \n"
                << std::endl;
      reportname = json_response["result"]["response"]["reportName"];
      std::cout << "    SupportReportName is " << json_response["result"]["response"]["reportName"] << "\n"
                << std::endl;
    }
    else
    {
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }

  std::cout << "Do supportReportGetPath. \n"
            << std::endl;
  str_response = "";

  //supportReportGetPath set json info
  Json request_getpath;
  request_getpath["id"] = 1;
  request_getpath["jsonrpc"] = "2.0";
  request_getpath["method"] = "supportReportGetPath";
  request_getpath["params"]["query"]["sessionId"] = session_id_;
  request_getpath["params"]["query"]["reportName"] = reportname;
  // post json info
  str_url = "http://" + host_name_ + ":8084";
  HttpClient::HttpPost(str_url, request_getpath.dump(), str_response, 300);

  //get error or necessary info
  Json json_response_getpath;
  try
  {
    json_response_getpath = Json::parse(str_response);
    int response_code = json_response_getpath["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    supportReportGetPath Success. \n"
                << std::endl;
      std::cout << "    supportReport Path is " << json_response_getpath["result"]["response"]["reportNameUrl"] << "\n"
                << std::endl;
      return true;
    }
    else
    {
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

//11.1.6 supportReportDelete
bool Bosch::supportReportDelete(std::string reportName)
{
  std::cout << "Do supportReportDelete. \n"
            << std::endl;
  std::string reportname = "";

  //supportReportDelete set json info
  Json request;
  request["id"] = 1;
  request["jsonrpc"] = "2.0";
  request["method"] = "supportReportDelete";
  request["params"]["query"]["sessionId"] = session_id_;
  request["params"]["query"]["reportName"] = reportName;

  // post json info
  std::string str_url = "http://" + host_name_ + ":8084";
  std::string str_response;
  HttpClient::HttpPost(str_url, request.dump(), str_response, 300);

  //get error or necessary info
  Json json_response;
  try
  {
    json_response = Json::parse(str_response);
    int response_code = json_response["result"]["response"]["responseCode"];
    if (response_code == 0)
    {
      std::cout << "    supportReportDelete Success. \n"
                << std::endl;
      return true;
    }
    else if (response_code == 11)
    {

      std::cout << "    supportReportDelete Failed (Code == 11). \n"
                << std::endl;
      std::cout << "    Tried to reference an entity does not exist. \n\n"
                << "    " << str_response << std::endl;
      return false;
    }
    else
    {
      std::cout << "    supportReportDelete Failed. \n"
                << str_response << std::endl;
      return false;
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "Failed to parse JSON data: " << e.what() << std::endl;
    return false;
  }
}

void testClientGlobalAlignAddObservation(Bosch bosch)
{
  ClientGlobalAlignObservation o;
  o.sensorName = "sensor";
  o.landmarkName = "landmark";
  o.laserStaticCalibSensor.x = 1;
  o.laserStaticCalibSensor.y = 2;
  o.laserStaticCalibSensor.a = 0.2;
  o.sensorLandmark.x = 1;
  o.sensorLandmark.y = 2;
  o.sensorLandmark.a = 0.2;
  o.mapLandmark.x = 1;
  o.mapLandmark.y = 2;
  o.mapLandmark.a = 0.2;
  o.obType = 1;
  o.caliType = 2;
  o.hasOrientation = true;
  bosch.clientGlobalAlignAddObservation(o);
}

void testClientGlobalAlignReplaceObservation(Bosch bosch)
{
  std::vector<ClientGlobalAlignObservation> vec;
  ClientGlobalAlignObservation o;
  o.sensorName = "sensor";
  o.landmarkName = "landmark";
  o.laserStaticCalibSensor.x = 1;
  o.laserStaticCalibSensor.y = 2;
  o.laserStaticCalibSensor.a = 0.2;
  o.sensorLandmark.x = 1;
  o.sensorLandmark.y = 2;
  o.sensorLandmark.a = 0.2;
  o.mapLandmark.x = 1;
  o.mapLandmark.y = 2;
  o.mapLandmark.a = 0.2;
  o.obType = 1;
  o.caliType = 2;
  o.hasOrientation = true;
  vec.push_back(o);
  bosch.clientGlobalAlignReplaceObservations("recordingName", vec);
}

//int main(int argc, char **argv)
//{
//  //create client
//  std::string hostName{"127.0.0.1"};
//  // init
//  Bosch bosch("127.0.0.1", "admin", "123");

//  //8.2.1 sessionLogin
//  bosch.sessionLogin(8080);

//  double x = 0;
//  double y = 0;
//  double angle = 0;
//  bosch.readConfigFile(x, y, angle);

//  bosch.clientLocalizationSwitch(false);
//  sleep(1);
//  bosch.clientLocalizationSwitch(true);
//  sleep(1);

//  bosch.runTcpTest(hostName, x, y, angle);

//  //save file
//  bosch.saveSeed(hostName);

//  return 0;
//}
