#pragma once
#include <qwidget.h>
#include "ChatSession.h"

class CChatTab : public QWidget
{
	Q_OBJECT
public:
	CChatTab(const CChatSessionPtr& pChatSession, QWidget* pParent = nullptr);

	CChatSessionPtr GetSession() { return m_pChatSession; }
	QPushButton* m_pSendButton;

private slots:
	void OnSend();
	void OnReceivedText(const QString& Text);
	void OnClosed();

private:
	QGridLayout* m_pLayout;
	QTextEdit* m_pChatLog;
	QLineEdit* m_pNewLine;


protected:
	CChatSessionPtr m_pChatSession;
};

