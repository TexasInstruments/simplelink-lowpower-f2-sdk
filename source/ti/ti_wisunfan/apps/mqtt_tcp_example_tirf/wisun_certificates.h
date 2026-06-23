/*
 * Copyright (c) 2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WISUN_TEST_CERTIFICATES_H_
#define WISUN_TEST_CERTIFICATES_H_

#ifndef RN_IOP_CERTIFICATE
const uint8_t WISUN_ROOT_CERTIFICATE[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIBLzCB1qADAgECAhQooDtB2DFWFsDlkVyeKLkkFq27vTAKBggqhkjOPQQDAjAN\r\n"
    "MQswCQYDVQQDEwJDQTAiGA8wMDAwMDEwMTAwMDAwMFoYDzk5OTkxMjMxMjM1OTU5\r\n"
    "WjANMQswCQYDVQQDEwJDQTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABG0ZABD+\r\n"
    "g8Nvc9KBpw/aHhoim9KrqzOYP5qTmZgS8uxM/eqeAJ6vrSivWFT2fxHuzXG+yfvs\r\n"
    "oPNgBJhv/YNhM9KjEDAOMAwGA1UdEwQFMAMBAf8wCgYIKoZIzj0EAwIDSAAwRQIg\r\n"
    "CK00/WNqrLzSt7QtustgbL+kibXBuWFeYBjU5yPbaHECIQD8aFwPt2/oXLOSa7gB\r\n"
    "9nwpA1rsmzCJLvb8ouxQ3uhXbg==\r\n"
    "-----END CERTIFICATE-----"
};

const uint8_t WISUN_SERVER_CERTIFICATE[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIBbzCCARUCFHsX/aO/8skZ/3NAQrzQ957H/aNYMAoGCCqGSM49BAMCMA0xCzAJ\r\n"
    "BgNVBAMTAkNBMCIYDzAwMDAwMTAxMDAwMDAwWhgPOTk5OTEyMzEyMzU5NTlaMGMx\r\n"
    "CzAJBgNVBAYTAkZJMQ0wCwYDVQQIDARPdWx1MQ0wCwYDVQQHDARPdWx1MQ0wCwYD\r\n"
    "VQQKDAR0ZXN0MQ0wCwYDVQQDDAR0ZXN0MRgwFgYJKoZIhvcNAQkBFgl0ZXN0QHRl\r\n"
    "c3QwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAQtbrqK+Z2gWz9rgvS4fgkWn1kb\r\n"
    "wERr15kFr7CmNM63gAmMyS08m5/sBYps5XUdIwF+Pz8uYmk4LpSl+o0OLkmaMAoG\r\n"
    "CCqGSM49BAMCA0gAMEUCIDknuOd/stYYgK3oztxspnKBHoOO8UcUB3PH2AfIq24e\r\n"
    "AiEA8OMJ44i3TKToinV1IATm81mqu+5pBqYu4RtX0E/xP88=\r\n"
    "-----END CERTIFICATE-----"
};

const uint8_t WISUN_SERVER_KEY[] = {
    "-----BEGIN EC PRIVATE KEY-----\r\n"
    "MHcCAQEEIEvbJ9SG/qeud6z3oRb+sROxk6/HWHQWtcucDFq3grzooAoGCCqGSM49\r\n"
    "AwEHoUQDQgAELW66ivmdoFs/a4L0uH4JFp9ZG8BEa9eZBa+wpjTOt4AJjMktPJuf\r\n"
    "7AWKbOV1HSMBfj8/LmJpOC6UpfqNDi5Jmg==\r\n"
    "-----END EC PRIVATE KEY-----"
};

const uint8_t WISUN_CLIENT_CERTIFICATE[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIBbzCCARUCFHsX/aO/8skZ/3NAQrzQ957H/aNZMAoGCCqGSM49BAMCMA0xCzAJ\r\n"
    "BgNVBAMTAkNBMCIYDzAwMDAwMTAxMDAwMDAwWhgPOTk5OTEyMzEyMzU5NTlaMGMx\r\n"
    "CzAJBgNVBAYTAkZJMQ0wCwYDVQQIDARPdWx1MQ0wCwYDVQQHDARPdWx1MQ0wCwYD\r\n"
    "VQQKDAR0ZXN0MQ0wCwYDVQQDDAR0ZXN0MRgwFgYJKoZIhvcNAQkBFgl0ZXN0QHRl\r\n"
    "c3QwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARIJ0hVdYmPsTFmY3glVAzE6dRE\r\n"
    "6Vp3rEUwqKqfMaJWvxd8EszaMP6PUwn4bprMkUPRmISTe8T17K8ZSRi8gun7MAoG\r\n"
    "CCqGSM49BAMCA0gAMEUCIQCtQwnVemwlIUoXeMZ1WE3JHj9XAIbxg2lweRJ91XaV\r\n"
    "VgIgDyft6+GF3u31VmTljTMAZZcCNRXDP0eNha+gB0TlGhY=\r\n"
    "-----END CERTIFICATE-----"
};

const uint8_t WISUN_CLIENT_KEY[] = {
    "-----BEGIN EC PRIVATE KEY-----\r\n"
    "MHcCAQEEIOxOq1xL+Hv5hg6Zg41pVXpkjLTkZxXrBHJcExTUMAftoAoGCCqGSM49\r\n"
    "AwEHoUQDQgAESCdIVXWJj7ExZmN4JVQMxOnUROlad6xFMKiqnzGiVr8XfBLM2jD+\r\n"
    "j1MJ+G6azJFD0ZiEk3vE9eyvGUkYvILp+w==\r\n"
    "-----END EC PRIVATE KEY-----"
};
#else

const uint8_t WISUN_ROOT_CERTIFICATE[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIBojCCAUmgAwIBAgIUSOJfgI08JDWdAjuqvH3REMyjjFswCgYIKoZIzj0EAwIw\r\n"
    "HjEcMBoGA1UEAwwTV2ktU1VOIERlbW8gUm9vdCBDQTAgFw0yMTAyMjIwOTU5NDFa\r\n"
    "GA85OTk5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTV2ktU1VOIERlbW8gUm9vdCBD\r\n"
    "QTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABG1Mn4dd9+IVJZSEcjpFKehvvRyQ\r\n"
    "t9QcIBCN2ysf+BJUlFfU8TvC3w2waFrLuC+JHM+1TBEm1GLNDF7piCgqltWjYzBh\r\n"
    "MBIGA1UdEwEB/wQIMAYBAf8CAQIwCwYDVR0PBAQDAgEGMB0GA1UdDgQWBBSTZNQO\r\n"
    "92ii7l3wrPpyvMUfTU86JDAfBgNVHSMEGDAWgBSTZNQO92ii7l3wrPpyvMUfTU86\r\n"
    "JDAKBggqhkjOPQQDAgNHADBEAiAdlM3ENdd7GHHbTsTiZMc7T5DDFQ2abeUI1be+\r\n"
    "ytGaAAIgZREIYV4yhjoluqT4+snj/zQkqEqcYh/DMbx2gLKDgZ4=\r\n"
    "-----END CERTIFICATE-----"
};

const uint8_t WISUN_SERVER_CERTIFICATE[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIBzzCCAXWgAwIBAgIUV9rWcXwDqRGrLVU/JRipf/q5ARUwCgYIKoZIzj0EAwIw\r\n"
    "HjEcMBoGA1UEAwwTV2ktU1VOIERlbW8gUm9vdCBDQTAgFw0yMTAzMDEwNzQyMTha\r\n"
    "GA85OTk5MTIzMTIzNTk1OVowJDEiMCAGA1UEAwwZV2ktU1VOIERlbW8gQm9yZGVy\r\n"
    "IFJvdXRlcjBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABGqBAsB47GbEpZbO3iDN\r\n"
    "aXeN9S2AEjMFUGyzoYvoVg/XUNhH5Z46eoVFIt8/+tfeI9Uha2xj0KKi9Cr3Q0Em\r\n"
    "P6qjgYgwgYUwDgYDVR0PAQH/BAQDAgOIMCEGA1UdJQEB/wQXMBUGCSsGAQQBguQl\r\n"
    "AQYIKwYBBQUHAwEwLwYDVR0RAQH/BCUwI6AhBggrBgEFBQcIBKAVMBMGCSsGAQQB\r\n"
    "grdBAQQGMTIzNDU2MB8GA1UdIwQYMBaAFJNk1A73aKLuXfCs+nK8xR9NTzokMAoG\r\n"
    "CCqGSM49BAMCA0gAMEUCIACT5SnUC+IRXrGNhX2XOursPvoGbKbpLyjtai3PwayX\r\n"
    "AiEAqxtUaEijpWJUby/RsX/yXLgD9/aATj9YFTR+ZdZ1VLo=\r\n"
    "-----END CERTIFICATE-----"
};

const uint8_t WISUN_SERVER_KEY[] = {
    "-----BEGIN PRIVATE KEY-----\r\n"
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgN0Zj70BWw1o/ZQWK\r\n"
    "fzslLkjUW4eUEMWZK5YII0aA0PahRANCAARqgQLAeOxmxKWWzt4gzWl3jfUtgBIz\r\n"
    "BVBss6GL6FYP11DYR+WeOnqFRSLfP/rX3iPVIWtsY9CiovQq90NBJj+q\r\n"
    "-----END PRIVATE KEY-----"
};

const uint8_t WISUN_CLIENT_CERTIFICATE[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIByDCCAW6gAwIBAgIUPRtrFcA6dw03sTpD1dArHpFi65gwCgYIKoZIzj0EAwIw\r\n"
    "HjEcMBoGA1UEAwwTV2ktU1VOIERlbW8gUm9vdCBDQTAgFw0yMTAzMDEwNzQyNDBa\r\n"
    "GA85OTk5MTIzMTIzNTk1OVowHTEbMBkGA1UEAwwSV2ktU1VOIERlbW8gRGV2aWNl\r\n"
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEqOqrunh3+gAr83a6wtpB+QhRvM4o\r\n"
    "9dTdVeNCo0amXhzbUsFylw7OgbgqtoCKQguYTsbicpeLMWv4TV7JxX6Ij6OBiDCB\r\n"
    "hTAOBgNVHQ8BAf8EBAMCA4gwIQYDVR0lAQH/BBcwFQYJKwYBBAGC5CUBBggrBgEF\r\n"
    "BQcDAjAvBgNVHREBAf8EJTAjoCEGCCsGAQUFBwgEoBUwEwYJKwYBBAGCt0ECBAYx\r\n"
    "MjM0NTYwHwYDVR0jBBgwFoAUk2TUDvdoou5d8Kz6crzFH01POiQwCgYIKoZIzj0E\r\n"
    "AwIDSAAwRQIhANBxFWMzNMKyA+nMK0sbCUpqK1gVMyeoKqh0zvS3COyLAiAx8nCN\r\n"
    "B7RkW8RmZ0UMWY26g7P6TbqJiAI3zoKkSxpJPg==\r\n"
    "-----END CERTIFICATE-----"
};

const uint8_t WISUN_CLIENT_KEY[] = {
    "-----BEGIN PRIVATE KEY-----\r\n"
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQguF1oBuIMzOtpsOMH\r\n"
    "df97vr2GppQfXOKDJ4RogFMk7QChRANCAASo6qu6eHf6ACvzdrrC2kH5CFG8zij1\r\n"
    "1N1V40KjRqZeHNtSwXKXDs6BuCq2gIpCC5hOxuJyl4sxa/hNXsnFfoiP\r\n"
    "-----END PRIVATE KEY-----\r\n"
};

#endif /* RN_IOP_CERTIFICATE */

#endif /* WISUN_TEST_CERTIFICATES_H_ */
