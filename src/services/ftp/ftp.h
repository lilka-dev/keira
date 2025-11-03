#include <FtpServer.h>
#include "services/network/network.h"
#include "keira/service.h"

#define FTP_USER            "lilka"
#define FTP_PASSWORD_LENGTH 6

class FTPService : public Service {
private:
    String user = FTP_USER;
    String password;
    NetworkService* networkService;
    FtpServer* ftpServer = nullptr;

public:
    FTPService();
    ~FTPService();

    String getUser();
    String getPassword();
    String getEndpoint();
    void createPassword();

private:
    void run() override;
};
