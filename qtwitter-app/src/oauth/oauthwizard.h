#ifndef OAUTHWIZARD_H
#define OAUTHWIZARD_H

#include <QtGui/QDialog>

namespace Ui {
  class OAuthWizard;
}

class QOAuth;

class OAuthWizard : public QDialog {
  Q_OBJECT
  Q_DISABLE_COPY(OAuthWizard)

public:
  static const QByteArray ConsumerKey;
  static const QByteArray ConsumerSecret;

  explicit OAuthWizard(QWidget *parent = 0);
  virtual ~OAuthWizard();

  QByteArray getScreenName() const;
  QByteArray getOAuthKey() const;
  bool authorized() const;

protected:
  virtual void changeEvent(QEvent *e);

private slots:
  void openUrl();
  void authorize();
  void setOkButtonEnabled();

private:
  bool state;
  QByteArray screenName;
  QByteArray token;
  QByteArray tokenSecret;

  QOAuth *qoauth;
  Ui::OAuthWizard *m_ui;

  static const QByteArray TwitterRequestTokenURL;
  static const QByteArray TwitterAccessTokenURL;
  static const QByteArray TwitterAuthorizeURL;
  static const QByteArray ParamCallback;
  static const QByteArray ParamCallbackValue;
  static const QByteArray ParamVerifier;
  static const QByteArray ParamScreenName;
  static const QByteArray ParamToken;
  static const QByteArray ParamTokenSecret;
};

#endif // OAUTHWIZARD_H
