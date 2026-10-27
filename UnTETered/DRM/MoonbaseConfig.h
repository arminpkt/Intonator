//
// Created by Armin Peukert on 20.10.26.
//

#pragma once

// Moonbase values from the Implementation guide in the Moonbase app.
// None of these are secret: the public key can only verify licenses.

namespace unTETered::moonbase_keys
{
    inline constexpr const char* endpoint  = "https://untetered.moonbase.sh";
    inline constexpr const char* productId = "untetered";
    inline constexpr const char* publicKey = R"(-----BEGIN RSA PUBLIC KEY-----
MIIBigKCAYEAxgCPFtEEYi5JYUNvmKN1yzm0zPENpyZ1CEuqcyK4L9p1ekZ4LwXO
+pQTj86gTh8qB8vBzkpGEyHey6rJqfbKUAAFK/WXNc4ECAPTj2ZzQYUT19sP3oXv
/Cq1feYJQFGWs5AnvC5mOaFBK5SmLWAQupMPVpI7tEDpaY01Jw+BaXRThwu1tLzx
ywHdgN8Fg28cZVU0HdhcBRyu+DocHC7lWsVN818SysV0Gb78ADDA7beL2b/hehLK
Cm/b/0GkhZMoC6Aa/pV+9pOUTetZGxwsds8sfwMsuqOHx8/nhPXo10nMknlIxqek
XH/jv5t7yBx/L+xC+dmHokKZ0HddHu5vSNNR0eP9B/Aqvy1hxcMCY9vhLBsFtc2B
wUkhi3YP+WIPoFzp4LuJ9AJOCOX2aWEGSa5BpdFVJt5dOuTsHTE/1eODOYWtC4Lo
jpWp8f5NJsAGFhmjsVw3bhq2sZlEkS3pSjMgt10o8YohVNlO2iZ3qurQvQyxj2nS
7lrv+83h+CeBAgMBAAE=
-----END RSA PUBLIC KEY-----)";
}