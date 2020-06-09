#pragma once
#include <QObject>
#include "wallet/common.h"

namespace grimmui
{
    QString toString(const grimm::wallet::WalletID&);
    QString toString(const grimm::Merkle::Hash&);
    QString GrimmToString(const grimm::Amount& value);
    QString toString(const grimm::Timestamp& ts);

    class Filter
    {
    public:
        Filter(size_t size = 12);
        void addSample(double value);
        double getAverage() const;
        double getMedian() const;
    private:
        std::vector<double> _samples;
        size_t _index;
    };
}