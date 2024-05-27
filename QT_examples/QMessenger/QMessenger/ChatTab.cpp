#include "stdafx.h"
#include "ChatTab.h"


CChatTab::CChatTab(const CChatSessionPtr& pChatSession, QWidget* pParent) : QWidget(pParent)
{
	m_pChatSession = pChatSession;

	connect(m_pChatSession.data(), &CChatSession::ReceivedText, this, &CChatTab::OnReceivedText);
	connect(m_pChatSession.data(), &CChatSession::Closed, this, &CChatTab::OnClosed);

	m_pLayout = new QGridLayout();
	this->setLayout(m_pLayout);
	m_pChatLog = new QTextEdit();
	m_pChatLog->setReadOnly(true);
	m_pLayout->addWidget(m_pChatLog, 0, 0);
	m_pNewLine = new QLineEdit();
	m_pNewLine->setPlaceholderText(tr("New Message"));
	m_pLayout->addWidget(m_pNewLine, 1, 0);
	connect(m_pNewLine, &QLineEdit::returnPressed, this, &CChatTab::OnSend);
	m_pSendButton = new QPushButton(tr("Send"));
	m_pLayout->addWidget(m_pSendButton, 1, 1);
	connect(m_pSendButton, &QPushButton::clicked, this, &CChatTab::OnSend);
}

void CChatTab::OnSend()
{
	if (m_pChatSession->SendText(m_pNewLine->text())) {

		m_pChatLog->append(tr("SEND: %1").arg(m_pNewLine->text()));

		m_pNewLine->clear();
	}
}

void CChatTab::OnReceivedText(const QString& Text)
{
	m_pChatLog->append(tr("RECV: %1").arg(Text));
}

void CChatTab::OnClosed()
{
	m_pChatLog->append(tr("+++"));
	m_pNewLine->setReadOnly(true);
}