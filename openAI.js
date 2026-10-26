
async function askOpenRouter() {

    const result =
        document.getElementById("result");


    result.textContent =
        " Đang gọi OpenRouter...";


    // API KEY MỚI CỦA BẠN
    const apiKey =
        "";


    try {

        const response =
            await fetch(

                "https://openrouter.ai/api/v1/chat/completions",

                {

                    method: "POST",


                    headers: {

                        "Content-Type":
                            "application/json",

                        "Authorization":
                            `Bearer ${apiKey}`

                    },


                    body: JSON.stringify({

                        model:
                            "recursive/flint-v1.0:free",


                        messages: [

                            {

                                role:
                                    "user",

                                content:
                                    "How many r's are in the word 'strawberry'?"

                            }

                        ],


                        stream:
                            true

                    })

                }

            );


        // ------------------------------------------------
        // KIỂM TRA HTTP
        // ------------------------------------------------

        if (!response.ok) {

            const errorText =
                await response.text();


            throw new Error(

                `OpenRouter API error: ${response.status}\n` +
                errorText

            );

        }


        // ------------------------------------------------
        // ĐỌC STREAM
        // ------------------------------------------------

        const reader =
            response.body.getReader();


        const decoder =
            new TextDecoder("utf-8");


        let responseText = "";


        while (true) {

            const {
                value,
                done
            } =
                await reader.read();


            if (done) {

                break;

            }


            const chunk =
                decoder.decode(

                    value,

                    {
                        stream: true
                    }

                );


            console.log(
                "RAW CHUNK:",
                chunk
            );


            // SSE
            const lines =
                chunk.split("\n");


            for (
                const line of lines
            ) {


                if (
                    !line.startsWith(
                        "data:"
                    )
                ) {

                    continue;

                }


                const data =
                    line
                        .substring(5)
                        .trim();


                if (
                    data === "[DONE]"
                ) {

                    continue;

                }


                try {

                    const json =
                        JSON.parse(
                            data
                        );


                    // Lấy nội dung AI
                    const content =

                        json
                            .choices?.[0]
                            ?.delta?.content;


                    if (content) {

                        responseText +=
                            content;


                        result.textContent =
                            responseText;


                        console.log(
                            content
                        );

                    }


                    // Usage
                    if (
                        json.usage
                    ) {

                        console.log(
                            "Usage:",
                            json.usage
                        );


                        console.log(

                            "Reasoning tokens:",

                            json
                                .usage
                                ?.completion_tokens_details
                                ?.reasoning_tokens

                        );

                    }


                }
                catch (error) {

                    console.log(
                        "Không parse được:",
                        data
                    );

                }

            }

        }


        // ------------------------------------------------
        // HOÀN THÀNH
        // ------------------------------------------------

        console.log(
            "Final response:",
            responseText
        );


        result.textContent =
            "✅ API hoạt động!\n\n" +
            responseText;


    }
    catch (error) {

        console.error(
            "OpenRouter Error:",
            error
        );


        result.textContent =
            "❌ Lỗi:\n\n" +
            error.message;

    }

}

