#include <Novice.h>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>

using namespace utility;              // 文字列変換などの一般的なユーティリティ
using namespace web;                  // URIのような共通の機能
using namespace web::http;            // 共通のHTTP機能
using namespace web::http::client;    // HTTP クライアントの機能
using namespace concurrency::streams; // 非同期ストリーム
web::json::value json;
web::json::value response;

template <class T>
pplx::task<T> Get(const std::wstring& url)
{
    return pplx::create_task([=]
                             {
                                 http_client client(url);
                                 return client.request(methods::GET); })
        .then([](http_response response)
              {
                  if (response.status_code() == status_codes::OK) {
                      return response.extract_json();
                  } });
}

pplx::task<int> Post(const std::wstring& url, int score)
{
    return pplx::create_task([=]
                             {
                                 json::value postData;
                                 postData[L"score"] = score;

                                 http_client client(url);
                                 return client.request(methods::POST, L"", postData.serialize(),
                                                       L"application/json"); })
        .then([](http_response response)
              {
                  if (response.status_code() == status_codes::OK) {
                      return response.extract_json();
                  } })
                                     .then(
                                         [](json::value json)
                                         { return json[L"serverStatus"].as_integer(); });
}

const char kWindowTitle[] = "RANKING";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // ライブラリの初期化
    Novice::Initialize(kWindowTitle, 1280, 720);

    // キー入力結果を受け取る箱
    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    int frameCount = 0;

    int score = 0;
    int isPlaying = false;

    // API通信が終わったかどうかのフラグ
    int wasScoreSent = false;

    // rankinkg
    int ranking[5] = { 0 };

    // ウィンドウの×ボタンが押されるまでループ
    while (Novice::ProcessMessage() == 0)
    {
        // フレームの開始
        Novice::BeginFrame();

        // キー入力を受け取る
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        ///
        /// ↓更新処理ここから
        ///

        // 入力処理
        if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0)
        {
            if (isPlaying)
            {
                isPlaying = false;
            }
            else
            {
                isPlaying = true;

                // 初期化
                if (frameCount != 0)
                {
                    frameCount = 0;
                    wasScoreSent = false;
                }
            }
        }

        // その他の更新処理
        if (isPlaying)
        {
            // ゲーム中
            frameCount++;

            // スコアの計算
            if (frameCount >= 600)
            {
                // 10秒を超えたら強制終了
                frameCount = 600;
                isPlaying = false;
            }
            else
            {
                score = frameCount * 10;
            }
        }
        else
        {
            if (frameCount > 0)
            {
                // APIに通信する
                if (!wasScoreSent)
                {
                    try
                    {
                        // Post
                        auto serverStatusCode =
                            Post(L"http://localhost:3000/swgames/", score).wait();
                        if (serverStatusCode == 1 || serverStatusCode == 2)
                        {
                            wasScoreSent = true;

                            // 投稿に成功したらランキングを取得
                            // Get
                            auto task =
                                Get<json::value>(L"http://localhost:3000/swgames/");
                            const json::value j = task.get();
                            auto array = j.as_array();

                            // JSONオブジェクトから必要部分を切り出してint型の配列に代入
                            for (int i = 0; i < array.size(); i++)
                            {
                                ranking[i] = array[i].at(U("score")).as_integer();
                            }
                            Novice::ConsolePrintf("SUCCESS\n");
                        }
                    }
                    catch (const std::exception& e)
                    {
                        Novice::ConsolePrintf("Error exception:%s\n", e.what());
                    }
                }
            }
        }

        ///
        /// ↑更新処理ここまで
        ///

        ///
        /// ↓描画処理ここから
        ///

        if (isPlaying)
        {
            // プレイ中
            Novice::ScreenPrintf(20, 320, "sec:");
            if (frameCount <= 400)
            {
                Novice::ScreenPrintf(20, 320, "    %f",
                                     static_cast<float>(frameCount) / 60);
            }
        }
        else
        {
            if (frameCount == 0)
            {
                // 開始前
                Novice::ScreenPrintf(20, 320, "STOP at 10.0sec. Press space to start.");
            }
            else
            {
                if (frameCount < 600)
                {
                    // 成功時
                    Novice::ScreenPrintf(20, 320, "Success!! Sec: %f Score:%d",
                                         static_cast<float>(frameCount) / 60, score);
                    Novice::ScreenPrintf(20, 340, "Press space to restart.");
                }
                else
                {
                    // 失敗時
                    Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0xD80000FF, kFillModeSolid);
                    Novice::ScreenPrintf(20, 320, "Burst!! Score:0");
                    Novice::ScreenPrintf(20, 340, "Press space to restart.");
                }
                Novice::ScreenPrintf(620, 100, "Ranking");

                for (int i = 0; i < 5; i++)
                {
                    Novice::ScreenPrintf(640, 120 + i * 20, "%d: %5d", i + 1, ranking[i]);
                }
            }
        }

        ///
        /// ↑描画処理ここまで
        ///

        // フレームの終了
        Novice::EndFrame();

        // ESCキーが押されたらループを抜ける
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0)
        {
            break;
        }
    }

    // ライブラリの終了
    Novice::Finalize();
    return 0;
}
