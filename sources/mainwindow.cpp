#include "mainwindow.h"
#include "startscreen.h"
#include "ui_mainwindow.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>

int MainWindow::kInstanceCount = 0;

MainWindow::MainWindow(int userId, QString userName, std::shared_ptr<Database> dbPtr, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_userId(userId),
  m_userName(userName)
{
  ui->setupUi(this);
  kInstanceCount++;
  if(dbPtr)
    m_dbPtr = dbPtr;
  else
    m_dbPtr = make_shared<Database>();
  auto timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &MainWindow::updateChats);
  timer->start(10);
  this->setWindowTitle("ChatWindow: " + userName);

  if((m_userName == "admin") || (m_userName == "root"))
  {
      QMessageBox msgBox;
      msgBox.setText("GOOD for you, " + userName);
      msgBox.exec();
  }
  else
  {
      ui->tabWidget->removeTab(2);
      ui->tabWidget->removeTab(1);
      setAttribute(Qt::WA_DeleteOnClose);
  }
}

MainWindow::~MainWindow()
{
  delete ui;
  kInstanceCount--;
  if(kInstanceCount <= 0)
    qApp->exit(0);
}

MainWindow *MainWindow::createClient(std::shared_ptr<Database> dbPtr)
{
  StartScreen s(dbPtr);
  auto result = s.exec();
  if(result == QDialog::Rejected)
  {
    return nullptr;
  }
  auto w = new MainWindow(s.userId(), s.userName(), s.getDatabase());
  w->setAttribute(Qt::WA_DeleteOnClose);
  return w;
}

void MainWindow::on_messageLineEdit_returnPressed()
{
    on_sendMessageButton_clicked();
}

void MainWindow::on_sendMessageButton_clicked()
{
  m_dbPtr->addChatMessage(m_userName.toStdString(),
                          ui->messageLineEdit->text().toStdString());
}

void MainWindow::on_privateMessageSendButton_clicked()
{
  QDialog dial(this);
  dial.setModal(true);
  auto l = new QVBoxLayout();
  dial.setLayout(l);
  auto userListWgt = new QListWidget(&dial);
  l->addWidget(userListWgt);
  auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dial);
  l->addWidget(buttonBox);

  connect(buttonBox, &QDialogButtonBox::accepted, &dial, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dial, &QDialog::reject);

  auto userList = m_dbPtr->getUserList();
  for(auto user : userList)
  {
    userListWgt->addItem(QString::fromStdString(user));
  }

  userListWgt->setCurrentRow(0);

  auto result = dial.exec();

  if(result == QDialog::Accepted &&
     userListWgt->currentItem())
  {
    m_dbPtr->addPrivateMessage(m_userName.toStdString(),
                               userListWgt->currentItem()->text().toStdString(),
                               ui->messageLineEdit->text().toStdString());
  }
}

void MainWindow::on_actionOpen_another_cliend_triggered()
{
  auto w = createClient(m_dbPtr);
  if(w)
    w->show();
}

void MainWindow::on_actionCloseClient_triggered()
{
  this->close();
}

void MainWindow::updateChats()
{
    auto chatMessages = m_dbPtr->getChatMessages();
    QString chat;
    for(const auto &msg : chatMessages)
    {
        chat.append(QString::fromStdString(msg) + "\n");
    }
    if(ui->commonChatBrowser->toPlainText() != chat)
        ui->commonChatBrowser->setText(chat);

    chat.clear();
    auto privateMessages = m_dbPtr->getPrivateMessage();
    QString prefix;
    for(const auto &msg : privateMessages)
    {
        if(QString::fromStdString(msg.getSender()) != m_userName &&
            msg.getDest() != m_userId)
        {
            continue;
        }

        std::string tmpStr = msg.getText();
        QString myT = QString::fromStdString(tmpStr);
        QString currCommand2 = "/BANN";
        if ((myT.contains(currCommand2, Qt::CaseInsensitive)) && ((msg.getSender() == "root") || (msg.getSender() == "admin")) && (m_userName != "admin") && (m_userName != "root") && (m_userId == msg.getDest()))
        {
            this->close();
        }

        if(m_userName == QString::fromStdString(msg.getSender()) &&
            m_userId == msg.getDest())
        {
            prefix = tr("self message") + ": ";
        }
        else if(m_userName == QString::fromStdString(msg.getSender()))
        {
            prefix = tr("message to") +
                     QString(" <%1>: ").
                     arg(QString::fromStdString(m_dbPtr->getUserName(msg.getDest())));
        }
        else
        {
            prefix = "<" + QString::fromStdString(msg.getSender()) +
                     "> " + tr("say to you") + ": ";
        }
        chat.append(prefix + QString::fromStdString(msg.getText()) + "\n");
    }
    if(ui->privateChatBrowser->toPlainText() != chat) ui->privateChatBrowser->setText(chat);
    chat.clear();

    QString prefix_2;
    for(const auto &msg : privateMessages)
    {
        if(msg.getDest() == m_dbPtr->searchUserByName(ui->label_21->text().toStdString()))
        {
            prefix_2 = "message From <" + QString::fromStdString(msg.getSender()) + "> ";
                    chat.append(prefix_2 + QString::fromStdString(msg.getText()) + "\n");
        }
    }
    if(ui->commonChatBrowser_2->toPlainText() != chat) ui->commonChatBrowser_2->setText(chat);
    chat.clear();

    QString prefix_3;
    for(const auto &msg : privateMessages)
    {

        if(QString::fromStdString(msg.getSender()) == QString::fromStdString(ui->label_21->text().toStdString()))
        {
            prefix_3 = "message To <" + QString::fromStdString(m_dbPtr->getUserName(msg.getDest())) + ">: ";
                    chat.append(prefix_3 + QString::fromStdString(msg.getText()) + "\n");
        }
    }
    if(ui->privateChatBrowser_2->toPlainText() != chat) ui->privateChatBrowser_2->setText(chat);
    chat.clear();

    QString prefix_4;
    for(const auto &msg : privateMessages)
    {
            prefix_4 = "messageFrom<" + QString::fromStdString(msg.getSender()) + ">" + "To<" + QString::fromStdString(m_dbPtr->getUserName(msg.getDest())) + ">: ";
                    chat.append(prefix_4 + QString::fromStdString(msg.getText()) + "\n");
    }
    if(ui->privateChatBrowser_3->toPlainText() != chat) ui->privateChatBrowser_3->setText(chat);
    chat.clear();
}

void MainWindow::on_pushButton_clicked()
{
    QDialog dial2(this);
    dial2.setModal(true);
    auto l = new QVBoxLayout();
    dial2.setLayout(l);
    auto userListWgt = new QListWidget(&dial2);
    l->addWidget(userListWgt);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dial2);
    l->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dial2, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dial2, &QDialog::reject);

    auto userList = m_dbPtr->getUserList();
    for(auto user : userList)
    {
      userListWgt->addItem(QString::fromStdString(user));
    }
    userListWgt->setCurrentRow(0);

    auto result = dial2.exec();
    if(result == QDialog::Accepted &&
       userListWgt->currentItem())
    {
      m_dbPtr->addPrivateMessage(m_userName.toStdString(),
                                 userListWgt->currentItem()->text().toStdString(),
                                 "/BANN");
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QDialog dial3(this);
    dial3.setModal(true);
    auto l = new QVBoxLayout();
    dial3.setLayout(l);
    auto userListWgt = new QListWidget(&dial3);
    l->addWidget(userListWgt);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dial3);
    l->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dial3, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dial3, &QDialog::reject);

    auto userList = m_dbPtr->getUserList();
    for(auto user : userList)
    {
      userListWgt->addItem(QString::fromStdString(user));
    }

    userListWgt->setCurrentRow(0);

    auto result = dial3.exec();
    if(result == QDialog::Accepted)
    {
        ui->label_21->setText(userListWgt->currentItem()->text());
    }
}
