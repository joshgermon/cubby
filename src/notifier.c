#include <curl/curl.h>
#include <stdio.h>
#include <time.h>

#define MAX_MSG_PAYLOAD 1024

static int send_slack_message(const char *message, const char *webhook_url)
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, webhook_url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    curl_global_cleanup();
    return 0;
}

int notify(const char *message, const char *webhook_url)
{
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    char message_payload[MAX_MSG_PAYLOAD];
    snprintf(message_payload, MAX_MSG_PAYLOAD,
    "{"
    "\"blocks\": ["
    "    {"
    "        \"type\": \"header\","
    "        \"text\": {"
    "            \"type\": \"plain_text\","
    "            \"text\": \":information_source: Cubby Backup :bear:\""
    "        }"
    "    },"
    "    {"
    "        \"type\": \"section\","
    "        \"text\": {"
    "            \"type\": \"mrkdwn\","
    "            \"text\": \"*Backup Process Started* "
    ":hourglass_flowing_sand:\\n\\nThe backup process has been initiated "
    "successfully.\""
    "        }"
    "    },"
    "    {"
    "        \"type\": \"section\","
    "        \"fields\": ["
    "            {"
    "                \"type\": \"mrkdwn\","
    "                \"text\": \"*Start Time:*\\n%04d-%02d-%02d "
    "%02d:%02d:%02d\""
    "            },"
    "            {"
    "                \"type\": \"mrkdwn\","
    "                \"text\": \"*Status:*\\nIn Progress "
    ":arrows_counterclockwise:\""
    "            }"
    "        ]"
    "    },"
    "    {"
    "        \"type\": \"divider\""
    "    },"
    "    {"
    "        \"type\": \"context\","
    "        \"elements\": ["
    "            {"
    "                \"type\": \"mrkdwn\","
    "                \"text\": \"If you have any questions, please contact the "
    "support team.\""
    "            }"
    "        ]"
    "    }"
    "]"
    "}",
    local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour,
    local->tm_min, local->tm_sec);


    send_slack_message(message_payload, webhook_url);

    return 0;
}
