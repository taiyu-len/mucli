#ifndef _MACRO_CURL_H
#define _MACRO_CURL_H
#include <stddef.h>

size_t
_nowrite(char *p, size_t s, size_t n, void *u);

// shitty Curl macros

// Requires `struct curl_httppost *post, *last`
#define curl_form(T1, name, T2, content) curl_formadd(&post, &last, \
	CURLFORM_##T1##NAME, name, CURLFORM_##T2##CONTENTS, content, CURLFORM_END)

// Requests `struct curl_httppost *post`
#define curl_post(curl) curl_easy_setopt(curl, CURLOPT_HTTPPOST, post)

#define curl_url(curl, url)  curl_easy_setopt(curl, CURLOPT_URL, url)

// Sets write function to nothing
#define curl_quiet(curl) curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _nowrite)

#define curl_share(curl) curl_easy_setopt(curl, CURLOPT_SHARE,\
		mucli.connection.curlsh)

#define curl_follow(curl) curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L)

#define curl_verbose(curl) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L)

#define curl_init(curl, url, code) \
		 ! (curl = curl_easy_init()) \
		|| (code = curl_share(curl)) \
		|| (code = curl_quiet(curl)) \
		|| (code = curl_verbose(curl)) \
		|| (code = curl_url(curl, url))

#define curl_response(curl,resp) \
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp)

#define curl_redirect(curl, url) \
	curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &url)

#endif

