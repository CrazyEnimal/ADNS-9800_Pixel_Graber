#include "mainwindow.h"
#include <QApplication>

#include <QSerialPort>
#include <QDebug>
#include <QSerialPortInfo>
#include "vimagewidget.h"
#include "hdlc_parser.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VImageWidget w;
    w.show();

    QSerialPortInfo portinfo;
    auto ports = QSerialPortInfo::availablePorts();
    for ( auto p: ports )
    {
        qDebug() << p.portName() << p.manufacturer() << p.productIdentifier() << p.vendorIdentifier();
        if (p.productIdentifier() == 29987 && p.vendorIdentifier() == 6790)
        {
            portinfo = p;
            break;
        }
    }

    if ( portinfo.isNull() )
    {
        qDebug() << "port not found";
        return 42;
    }

    QSerialPort com( portinfo );
    auto ok = com.open( QIODevice::ReadWrite );
    if (!ok) qDebug() << com.errorString();
    Q_ASSERT( ok );

    //ok = com.setBaudRate( 250000 );
    ok = com.setBaudRate( 250000 );
    if (!ok) qDebug() << com.errorString();
    Q_ASSERT( ok );

    HDLC_Parser parser;

    QObject::connect( &com, &QSerialPort::readyRead, [&]()
    {
        auto data = com.readAll();
        //vdeb << data.size() << data.toHex();
        parser.append( data.toStdString() );
    });

    parser.received = [&]( const std::string& str )
    {
        QImage img( 30, 30, QImage::Format_Grayscale8 );
        auto pixel = str.begin();
        for ( auto r = 0; r < 30; ++r )
        {
            for ( auto c = 0; c < 30; ++c )
            {
                auto line = img.scanLine( c );
                line[r] = uchar(*pixel++);
            }
        }
        w.set_image( img );
//        vdeb << str.size();
//        vdeb << str.to_Hex();
    };

    return a.exec();
}
