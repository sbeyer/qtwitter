/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#include <QPixmap>
#include <QDebug>
#include "statuslistview.h"
#include "statuslist.h"
#include "tweetmodel.h"
#include "tweet.h"
#include "settings.h"

TweetModel::TweetModel( StatusListView *parentListView, QObject *parent ) :
    QStandardItemModel( 0, 0, parent ),
    isVisible( false ),
    maxTweetCount( 20 ),
    statusList(0),
    currentIndex( QModelIndex() ),
    view( parentListView )
{
  connect( view, SIGNAL(clicked(QModelIndex)), this, SLOT(selectTweet(QModelIndex)) );
  connect( view, SIGNAL(moveFocus(bool)), this, SLOT(moveFocus(bool)) );
}

void TweetModel::populate()
{
  if ( rowCount() < maxTweetCount ) {
    for ( int i = rowCount(); i < maxTweetCount; ++i ) {
      Tweet *tweet = new Tweet( this );
      QStandardItem *newItem = new QStandardItem;
      newItem->setSizeHint( tweet->size() );
      appendRow( newItem );
      view->setIndexWidget( newItem->index(), tweet );
    }
  } else {
    removeRows( maxTweetCount, rowCount() - maxTweetCount );
  }
}

void TweetModel::updateDisplay()
{
  // statusList is meant to be the same size as model ( rowCount() ),
  // but at the beginning we do have the statusList, but it's empty,
  // so rowCount() would fail
  if ( statusList ) {
    for ( int i = 0; i < statusList->size(); ++i ) {
      updateDisplay( i );
    }
  } else {
    for ( int i = 0; i < rowCount(); ++i ) {
      updateDisplay( i );
    }
  }
  // FIXME: Sorry, I know that it sucks... :|
  view->setUpdatesEnabled( false );
  view->resize( view->width(), view->height() - 1 );
  view->resize( view->width(), view->height() + 1 );
  view->setUpdatesEnabled( true );
}

void TweetModel::updateDisplay( int ind )
{
  Tweet *tweet = static_cast<Tweet*>( view->indexWidget( index( ind, 0 ) ) );
  Q_ASSERT(tweet);
  if ( statusList )
    tweet->setTweetData( statusList->data( ind ) );
  item( ind )->setSizeHint( tweet->size() );
}

void TweetModel::updateImage( int ind )
{
  Tweet *tweet = static_cast<Tweet*>( view->indexWidget( index( ind, 0 ) ) );
  Q_ASSERT(tweet);
  tweet->setImage( statusList->data( ind ).image );
}

void TweetModel::updateState( int ind )
{
  Tweet *tweet = static_cast<Tweet*>( view->indexWidget( index( ind, 0 ) ) );
  Q_ASSERT(tweet);
  tweet->setState( statusList->data( ind ).state );
}

Tweet* TweetModel::currentTweet()
{
  if ( !currentIndex.isValid() )
    return 0;

  return static_cast<Tweet*>( view->indexWidget( currentIndex ) );
}

void TweetModel::deselectCurrentIndex()
{
  if ( currentIndex.isValid() ) {
    Tweet *tweet = static_cast<Tweet*>( view->indexWidget( currentIndex ) );
    Q_ASSERT(tweet);
    TweetModel::TweetState state = statusList->state( currentIndex.row() );
    if ( state != TweetModel::STATE_UNREAD ) {
      state = TweetModel::STATE_READ;
      statusList->setState( currentIndex.row(), state );
    }
    tweet->setTweetData( statusList->data( currentIndex.row() ) );
    currentIndex = QModelIndex();
  }
}

void TweetModel::setTheme( const ThemeData &theme )
{
  Tweet::setTheme( theme );
  Tweet *tweet;
  for ( int i = 0; i < rowCount(); i++ ) {
    tweet = static_cast<Tweet*>( view->indexWidget( index( i, 0 ) ) );
    Q_ASSERT(tweet);
    tweet->applyTheme();
  }
}

void TweetModel::setStatusList( StatusList *statusList )
{
  if ( this->statusList ) {
    disconnect( this->statusList, SIGNAL(statusAdded(int)), this, SLOT(updateDisplay(int)) );
    disconnect( this->statusList, SIGNAL(dataChanged(int)), this, SLOT(updateDisplay(int)) );
    disconnect( this->statusList, SIGNAL(stateChanged(int)), this, SLOT(updateState(int)) );
    disconnect( this->statusList, SIGNAL(imageChanged(int)), this, SLOT(updateImage(int)) );
  }

  this->statusList = statusList;
  connect( this->statusList, SIGNAL(statusAdded(int)), this, SLOT(updateDisplay(int)) );
  connect( this->statusList, SIGNAL(dataChanged(int)), this, SLOT(updateDisplay(int)) );
  connect( this->statusList, SIGNAL(stateChanged(int)), this, SLOT(updateState(int)) );
  connect( this->statusList, SIGNAL(imageChanged(int)), this, SLOT(updateImage(int)) );

  Tweet::setCurrentLogin( this->statusList->login() );
  Tweet::setCurrentNetwork( this->statusList->network() );
  updateDisplay();
}

StatusList * TweetModel::getStatusList() const
{
  return statusList;
}

void TweetModel::setMaxTweetCount( int count )
{
  if ( statusList && count < maxTweetCount )
    statusList->remove( count, maxTweetCount - count );
  maxTweetCount = count;
  populate();
}

void TweetModel::clear()
{
  if ( !statusList )
    return;

  statusList = 0;

  Tweet *tweet;
  for ( int i = 0; i < rowCount(); ++i )
  {
    tweet = static_cast<Tweet*>( view->indexWidget( index( i, 0 ) ) );
    Q_ASSERT(tweet);
    tweet->initialize();
  }
  updateDisplay();
}

void TweetModel::sendDeleteRequest( int id )
{
  qDebug() << "TweetModel::sendDeleteRequest";
  emit destroy( statusList->network(), statusList->login(), id );
}

//void TweetModel::slotDirectMessagesChanged( bool isEnabled )
//{
//  if ( isEnabled )
//    return;
//  for ( int i = 0; i < rowCount(); i++ ) {
//    if ( statuses[i].entry.type == Entry::DirectMessage ) {
//      if ( isVisible ) {
//        delete view->indexWidget( item(i)->index() );
//      } else {
//        delete statuses[i].tweet;
//      }
//      removeRow(i);
//      Q_ASSERT(statuses[i].tweet == 0 );
//      statuses.removeAt(i);
//      i--;
//    }
//  }
//}

void TweetModel::selectTweet( const QModelIndex &index )
{
  if ( !statusList || !index.isValid() )
    return;

  TweetModel::TweetState state;
  Tweet *tweet;

  if ( currentIndex.isValid() ) {
    state = statusList->state( currentIndex.row() );
    if ( state != TweetModel::STATE_UNREAD ) {
      state = TweetModel::STATE_READ;
      statusList->setState( currentIndex.row(), state );
    }

    tweet = static_cast<Tweet*>( view->indexWidget( currentIndex ) );
    Q_ASSERT(tweet);
    tweet->setTweetData( statusList->data( currentIndex.row() ) );
  }

  currentIndex = index;

  statusList->setState( index.row(), TweetModel::STATE_ACTIVE );

  tweet = static_cast<Tweet*>( view->indexWidget( currentIndex ) );
  Q_ASSERT(tweet);
  tweet->setTweetData( statusList->data( currentIndex.row() ) );

  view->setCurrentIndex( currentIndex );
}

void TweetModel::selectTweet( Tweet *tweet )
{
  if ( !statusList || tweet->getState() == TweetModel::STATE_DISABLED )
    return;

  Status status;
  if ( currentIndex.isValid() && currentIndex.row() < statusList->size() ) {
    status = statusList->data( currentIndex.row() );
    if ( status.state != TweetModel::STATE_UNREAD ) {
      statusList->setState( currentIndex.row(), TweetModel::STATE_READ );
    }
  }
  for ( int i = 0; i < statusList->size(); i++ ) {
    status = statusList->data( i );
    if ( status.entry.id == tweet->getId() ) {
      currentIndex = item(i)->index();
      statusList->setState( currentIndex.row(), TweetModel::STATE_ACTIVE );
      view->setCurrentIndex( currentIndex );
    }
  }
}

void TweetModel::markAllAsRead()
{
  if ( rowCount() > 0 ) {
    Status status;
    for ( int i = 0; i < rowCount(); i++ ) {
      status = statusList->data(i);
      if ( i == currentIndex.row() )
        status.state = TweetModel::STATE_ACTIVE;
      else
        status.state = TweetModel::STATE_READ;
      statusList->setState(i, status.state );
    }
  }
}

// TODO: consider moving to StatusList
void TweetModel::checkForUnread()
{
  qDebug() << "TweetModel::checkForUnread(" << login << ");";
  for ( int i = 0; i < rowCount(); ++i ) {
    if ( statusList->data(i).state == TweetModel::STATE_UNREAD ) {
      emit newTweets( login, true );
      return;
    }
  }
  emit newTweets( login, false );
}

void TweetModel::retranslateUi()
{
  return;
  Tweet *tweet;
  for ( int i = 0; i < statusList->size(); i++ ) {
    tweet = static_cast<Tweet*>( view->indexWidget( index( i, 0 ) ) );
    tweet->retranslateUi();
  }
}

void TweetModel::resizeData( int width, int oldWidth )
{
  Tweet *tweet;
  for ( int i = 0; i < rowCount(); i++ ) {
    tweet = static_cast<Tweet*>( view->indexWidget( index( i, 0 ) ) );
    tweet->resize( width, tweet->size().height() );
    item(i)->setSizeHint( tweet->size() );
  }
}

void TweetModel::moveFocus( bool up )
{
  if ( !rowCount() )
    return;
  if ( !currentIndex.isValid() ) {
    currentIndex = index( 0, 0 );
    selectTweet( currentIndex );
    return;
  }
  if ( up ) {
    if ( currentIndex.row() > 0 ) {
      selectTweet( currentIndex.sibling( currentIndex.row() - 1, 0 ) );
    }
  } else {
    if ( currentIndex.row() < rowCount() - 1 ) {
      selectTweet( currentIndex.sibling( currentIndex.row() + 1, 0 ) );
    }
  }
}

void TweetModel::setImageForUrl( const QString& url, QPixmap *image )
{
  Status status;
  Tweet *tweet;
  for ( int i = 0; i < statusList->size(); i++ ) {
    status = statusList->data(i);
    if ( url == status.entry.image ) {
      status.image = *image;
      tweet = static_cast<Tweet*>( view->indexWidget( index( i, 0 ) ) );
      tweet->setTweetData( status );
    }
  }
}

void TweetModel::emitOpenBrowser( QString address )
{
  emit openBrowser( QUrl( address ) );
}


/*! \class TweetModel
  \brief A class for managing the Tweet list content.

  This class contains a model for a list view displaying status updates.
  It is responsible for behind the scenes management of all the actions
  like adding, sorting, deleting and updating Tweets. Furthermore since
  it is directly connected with all the Tweets, it manages their selecting
  and deselecting.
*/

/*! \fn TweetModel::TweetModel( int margin, StatusListView *parentListView, QObject *parent = 0 )
    Creates a tweet list model with a given \a parent.
    \param margin Holds the width of the Tweet list's scrollbar useful when setting
                  size of the Tweet widgets.
    \param parentListView The list view that the model serves for.
    \param parent A parent for the tweet list model.
*/

/*! \fn void TweetModel::deselectCurrentIndex();
    Removes selection from currently highlighted item.
    \sa selectTweet()
*/

/*! \fn void TweetModel::setTheme( const ThemeData &theme )
    Sets \a theme to be the current theme for all the Tweets.
    \param theme The theme to be set.
*/

/*! \fn void TweetModel::setMaxTweetCount( int count )
    Sets maximum amount of Tweets on a list.
    \param count The given maximum Tweets amount.
*/

/*! \fn void TweetModel::insertTweet( Entry *entry )
    Creates a Tweet class instance as a representation of \a entry and adds it
    to the list in an appropriate place (sorting chronogically).
    \param entry The entry on which the new Tweet bases.
    \sa deleteTweet()
*/

/*! \fn void TweetModel::deleteTweet( int id )
    Removes Tweet of the given id from the model and deletes it.
    \param id An id of the Tweet to be deleted.
    \sa insertTweet()
*/

/*! \fn void TweetModel::slotDirectMessagesChanged( bool isEnabled )
    Removes all direct messages from the model and deletes them. Used when
    User disables direct messages download.
*/

/*! \fn void TweetModel::selectTweet( const QModelIndex &index )
    Highlights a Tweet of a given \a index. Used by a mouse press event on the
    Tweet list view.
    \param index The model index of the element to be selected.
    \sa deselectCurrentIndex()
*/

/*! \fn void TweetModel::selectTweet( Tweet *tweet )
    Highlights a given Tweet. Used by a mouse press event on the Tweet's
    internal status display widget.
    \param tweet A Tweet to be selected.
    \sa deselectCurrentIndex()
*/

/*! \fn void TweetModel::markAllAsRead()
    Sets all Tweets' state to TweetModel::STATE_READ.
*/

/*! \fn void TweetModel::sendNewsInfo()
    Counts unread Tweets and messages and emits newTweets() signal
    to notify MainWindow about a tray icon message to pop up.
*/

/*! \fn void TweetModel::retranslateUi()
    Retranslates all Tweets.
*/

/*! \fn void TweetModel::resizeData( int width, int oldWidth )
    Resizes Tweets according to given values.
    \param width New width of MainWindow.
    \param oldWidth Old width of MainWindow.
*/

/*! \fn void TweetModel::moveFocus( bool up )
    Selects the current Tweet's neighbour, according to the given \a up parameter.
    \param up Selects upper Tweet if true, otherwise selects lower one.
    \sa selectTweet()
    \sa deselectCurrentIndex()
*/

/*! \fn void TweetModel::setImageForUrl( const QString& url, QPixmap image )
    Assigns the given \a image to all the Tweets having \a url as their profile image URL.
    \param url Profile image URL of the Tweet.
    \param image Image to be set for the Tweet(s).
*/

/*! \fn void TweetModel::setModelToBeCleared( bool wantsPublic, bool userChanged )
    Evaluates an internal flag that indicates if a model has to be completely
    cleared before inserting new Tweets. This occurs e.g. when switching from
    public to friends timeline, or when synchronising with friends timeline
    and changing authenticating user.
    \param wantsPublic Indicates if public timeline will be requested
                       upon next update.
    \param userChanged Indicates if the authentcating user has changed since the
                       last update.
*/

/*! \fn void TweetModel::setPublicTimelineRequested( bool b )
    Sets the flag indicating if the public timeline will be requested upon the
    next connection.
    \param b The new flag's value.
*/

/*! \fn void TweetModel::retweet( QString message )
    Passes the retweet message from a particular Tweet to the MainWindow.
    \param message Retweet status message.
    \sa reply()
*/

/*! \fn void TweetModel::destroy( int id )
    A request to destroy a Tweet, passed from a specific Tweet to the Core class instance.
    \param id The Tweet's id.
*/

/*! \fn void TweetModel::newTweets( int statusesCount, QStringList statusesNames, int messagesCount, QStringList messagesNames )
    Emitted by \ref sendNewsInfo() to notify MainWindow of a new statuses.
    \param statusesCount Amount of the new statuses.
    \param statusesNames List of new statuses authors.
    \param messagesCount Amount of the new direct messages.
    \param messagesNames List of new direct messages authors.
*/

/*! \fn void TweetModel::openBrowser( QUrl address )
    Emitted to pass the request to open web browser to the Core class instance.
    \param address Web address to be accessed.
*/

/*! \fn void TweetModel::reply( const QString &name, int inReplyTo )
    Passes the reply request from a particular Tweet to the MainWindow.
    \param name Login of the original message author.
    \param inReplyTo Id of the existing status to which the reply is posted.
    \sa retweet()
*/

/*! \fn void TweetModel::about()
    Passes the request to popup an about dialog to the MainWindow.
*/
