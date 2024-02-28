#include <curl/curl.h>
#include <stdio.h>

#define MAX_MSG_PAYLOAD 256

static int send_slack_message(const char *message)
{
    CURL *curl;
    CURLcode res;

    // Get from a global configuration store
    static const char *webhookUrl = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, webhookUrl);
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

int notify(char *message)
{
  // Once config implented can check config for notification service
  // if ( config->notifier == SLACK_NOTIFIER )
  // else if ( config->notifier == DISCORD_NOTIFIER )
  char message_payload[MAX_MSG_PAYLOAD];
  snprintf(message_payload, MAX_MSG_PAYLOAD, "{\"text\":\"%s\"}", message);

  send_slack_message(message_payload);

  return 0;
}
