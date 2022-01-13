#pragma once
// 현재시간을 string type으로 return하는 함수
#include <string>
#include <time.h>

const std::string currentDateTime() {
    time_t     now = time(0); //현재 시간을 time_t 타입으로 저장
    struct tm  tstruct;
    char       buf[40];
    localtime_s(&tstruct, &now);
    strftime(buf, sizeof(buf), "%Y%m%d", &tstruct); // YYYYMMDD 형태의 스트링

    return buf;
}