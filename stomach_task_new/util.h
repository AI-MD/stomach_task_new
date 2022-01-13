#pragma once
// ����ð��� string type���� return�ϴ� �Լ�
#include <string>
#include <time.h>

const std::string currentDateTime() {
    time_t     now = time(0); //���� �ð��� time_t Ÿ������ ����
    struct tm  tstruct;
    char       buf[40];
    localtime_s(&tstruct, &now);
    strftime(buf, sizeof(buf), "%Y%m%d", &tstruct); // YYYYMMDD ������ ��Ʈ��

    return buf;
}