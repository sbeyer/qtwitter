/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef IMAGEDOWNLOAD_H
#define IMAGEDOWNLOAD_H

#include "httpconnection.h"
#include <QMap>

struct ImageData {
  QImage *img;
  QByteArray *bytearray;
  QBuffer *buffer;
};

class ImageDownload : public HttpConnection {
  Q_OBJECT
public:
  ImageDownload();
  ~ImageDownload();
  void syncGet( const QString& path, bool isSync = false );
  void imgGet( const Entry &entry );
  QImage getUserImage();

private:
  QImage *userImage;
  QEventLoop getEventLoop;
  QMap<QString,int> requestByEntry;
  QMap<QString,ImageData> imageByEntry;

public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void httpRequestStarted( int requestId );
  
signals:  
  void imageDownloaded( const QString&, QImage );
  void imageReadyForUrl( const QString&, QImage );
};

#endif //IMAGEDOWNLOAD_H
