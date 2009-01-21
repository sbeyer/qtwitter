#ifndef CORE_H
#define CORE_H

#include <QStandardItemModel>

#include "entry.h"
#include "xmldownload.h"
#include "imagedownload.h"

typedef QList<Entry> ListOfEntries;
typedef QMap<QString, QImage> MapStringImage;

class Core : public QThread {
  Q_OBJECT

public:
  Core();
  virtual ~Core();
  void get( const QString &path );
  void post( const QString &path, const QByteArray &status );

protected:
  void run();

public slots:

  void addEntry( const Entry &entry, int type );
  void saveImage( const QString &imageUrl, QImage image );
  void downloadImages();
  void error( const QString &message );
    
signals:
  void readyToDisplay( const ListOfEntries &entries, const MapStringImage &imagesHash );
  void errorMessage( const QString &message );
  
private:
  bool xmlBeingProcessed;
  XmlDownload xmlGet;
  XmlDownload xmlPost;
  ImageDownload *imageDownload;
  MapStringImage imagesHash;
  ListOfEntries entries;
  QStandardItemModel model;
};


#endif //CORE_H
