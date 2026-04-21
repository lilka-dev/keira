#include <FtpServer.h>
#include "services/network/network.h"
#include "keira/service.h"
#include "keira/mutex.h"
#include "keira/config.h"

#define FTP_USER            "lilka"
#define FTP_PASSWORD_LENGTH 6

class FTPService : public Service {
private:
    String user = FTP_USER;
    String password;
    NetworkService* networkService = NULL;
    FtpServer* ftpServer = nullptr;
    SemaphoreHandle_t ftpMtx = xSemaphoreCreateMutex();

public:
    ~FTPService();

    KMTX_GETER(String, user, ftpMtx);
    KMTX_SETER_GETER(String, password, ftpMtx);
    void createPassword();

    void onStart() override;

private:
    void run() override;
};
