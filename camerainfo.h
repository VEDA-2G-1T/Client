#ifndef CAMERAINFO_H
#define CAMERAINFO_H

#include <QString>

struct CameraInfo {
    QString name;
    QString ip;
    QString port;

    QString rtspUrl() const {
        return QString("rtsps://%1:%2/raw").arg(ip, port);
    }

    bool operator==(const CameraInfo &other) const {
        return name == other.name && ip == other.ip && port == other.port;
    }
};

#endif // CAMERAINFO_H
