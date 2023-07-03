#include <cpprest/filestream.h>
#include <cpprest/http_client.h>

using namespace utility;              // 文字列変換などの一般的なユーティリティ
using namespace web;                  // URIのような共通の機能
using namespace web::http;            // 共通のHTTP機能
using namespace web::http::client;    // HTTP クライアントの機能
using namespace concurrency::streams; // 非同期ストリーム

pplx::task<void> GetTest();

int main() {
	try
	{
		GetTest().wait();
	}
	catch (const std::exception& e)
	{
		printf("Error exception:%s\n", e.what());
	}

	return 0;
};

pplx::task<void> GetTest() {
	return pplx::create_task([] {
		// クライアントの設定
		http_client client(L"http://localhost:3000/faculty");

		// リクエスト送信
		return client.request(methods::GET);
							 })
		.then([](http_response response) {
								 // ステータスコードの表示
								 if (response.status_code() == status_codes::OK) {
									 // レスポンスボディを表示
									 auto body = response.extract_string();
									 setlocale(LC_CTYPE, "");
									 std::wcout << body.get().c_str() << std::endl;
								 }
			  });
}
