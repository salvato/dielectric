#ifndef HP4284A_H
#define HP4284A_H

#include <QObject>

class Hp4284a : public QObject
{
    Q_OBJECT
public:
    explicit Hp4284a(int address, QObject *parent = nullptr);

signals:

public slots:

private:
  int hp4284aAddress;
  int hp4284a;
  char spollByte;
};

#endif // HP4284A_H
