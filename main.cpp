#include <cpprest/filestream.h>
#include <cpprest/http_client.h>

using namespace utility;              // 文字列変換などの一般的なユーティリティ
using namespace web;                  // URIのような共通の機能
using namespace web::http;            // 共通のHTTP機能
using namespace web::http::client;    // HTTP クライアントの機能
using namespace concurrency::streams; // 非同期ストリーム

template <class T>
pplx::task<T> Get(const std::wstring& url);
pplx::task<int> Post(const std::wstring& url, const std::wstring& name = L"test");

int main() {
	printf("使用したい機能を選んでください\n");
	printf("1:表示、2:追加\n");
	int ascii = getchar();
	printf("\n");

	setlocale(LC_CTYPE, "");
	try
	{
		switch (ascii)
		{
		case '1':
		{// Get
			auto task = Get<json::value>(L"http://localhost:3000/faculty");
			const json::value j = task.get();
			auto jArray = j.as_array();
			for (int i = 0; i < jArray.size(); i++)
			{
				std::wcout << jArray[i] << std::endl;
			}

			break;
		}
		case '2':
		{// Post
			std::wstring name;
			std::wcin >> name;

			auto serverStatusCode = Post(L"http://localhost:3000/faculty", name).get();
			if (serverStatusCode == 1 || serverStatusCode == 2)
			{
				std::cout << "新規データを追加しました。" << std::endl;
			}

			break;
		}
		default:
			printf("その機能はありません\n");
			break;
		}
	}
	catch (const std::exception& e)
	{
		printf("Error exception:%s\n", e.what());
	}

	printf("\n");
	system("pause");
	return 0;
};

template <class T>
pplx::task<T> Get(const std::wstring& url)
{
	return pplx::create_task([=]
							 {
								 // クライアントの設定
								 http_client client(url);

								 // リクエスト送信
								 return client.request(methods::GET);
							 })
		.then([](http_response response)
			  {
				  // ステータスコードの表示
				  if (response.status_code() == status_codes::OK)
				  {
					  return response.extract_json();
				  }
			  });
}

pplx::task<int> Post(const std::wstring& url, const std::wstring& name)
{
	return pplx::create_task([=]
							 {
								 // 送信データの作成
								 json::value postData;
								 postData[L"name"] = json::value::string(name);

								 // クライアントの設定
								 http_client client(url);

								 // リクエスト送信
								 return client.request(methods::POST, L"", postData.serialize(), L"application/json");
							 })
		.then([](http_response response)
			  {
				  // ステータスコードの表示
				  if (response.status_code() == status_codes::OK)
				  {
					  return response.extract_json();
				  }
			  })
		.then([](json::value json)
			  {
				  return json[L"serverStatus"].as_integer();
			  });
}
