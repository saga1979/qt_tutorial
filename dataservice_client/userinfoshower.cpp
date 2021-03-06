#include "userinfoshower.h"
#include "ui_userinfoshower.h"

#include "userinfomanager.h"
#include "datamanager.h"
#include <QFileDialog>
#include <QImageReader>
#include <QSplitter>

#include <QApplication>


UserInfoShower::UserInfoShower(QWidget *parent, UserInfoShowerMode mode) :
    QWidget(parent),
    ui(new Ui::UserInfoShower),
    m_userInfo(0)
{
    ui->setupUi(this);
    ui->le_file->setReadOnly(true);
    ui->bt_upload->setDisabled(true);
    ui->lb_img->setFixedSize(QSize(100,100));
    m_mode = mode;

    connect(ui->bt_browse, SIGNAL(clicked(bool)), this, SLOT(onBrowseClick()));
    connect(ui->bt_upload, SIGNAL(clicked(bool)), this, SLOT(onUploadClick()));
    connect(ui->le_user, SIGNAL(textChanged(QString)), this, SLOT(onUserNameChanged(QString)));

    if(mode != UserInfoShowerMode::Adding)
    {
        ui->le_user->setEnabled(false);
    }



    DataManager * pdm = new DataManager(this);
    ui->layout_main->addWidget(pdm);

}

UserInfoShower::~UserInfoShower()
{
    delete ui;
    if(m_userInfo !=0)
    {
        delete m_userInfo;
    }
}

void UserInfoShower::setUserInfo(UserInfo *userInfo)
{
    if(m_userInfo !=0)
    {
        delete m_userInfo;
    }
    m_userInfo = new UserInfoEx;

    m_userInfo->name.second = userInfo->name;
    m_userInfo->salary.second = userInfo->salary;
    m_userInfo->pic.second = userInfo->pic;

    //update ui
    if(!m_userInfo->pic.second.empty())
    {
        QPixmap pix;
        pix.loadFromData((const uchar*)m_userInfo->pic.second.c_str(),m_userInfo->pic.second.length());
        ui->lb_img->setPixmap(pix.scaled(QSize(ui->lb_img->width(), ui->lb_img->height())));
    }
    ui->le_user->setText(QString::fromStdString(userInfo->name));
}

const UserInfoEx *UserInfoShower::getUserInfoEx() const
{
    return m_userInfo;
}

const UserInfo& UserInfoShower::getUserInfo() const
{
    static UserInfo user;
    user.name = m_userInfo->name.second;
    user.salary = m_userInfo->salary.second;
    user.pic = m_userInfo->pic.second;

    return user;
}

void UserInfoShower::onBrowseClick()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
            tr("image files (*.png)"));

    if(fileName.isEmpty())
    {
        return;
    }

    ui->le_file->setText(fileName);

    QImageReader ir(fileName);
    QPixmap pix = QPixmap::fromImageReader(&ir);

    ui->lb_img->setPixmap(pix.scaled(QSize(ui->lb_img->width(), ui->lb_img->height())));
    ui->bt_upload->setDisabled(false);
    m_userInfo->pic.first = true;

}

void UserInfoShower::onUploadClick()
{
    if(m_mode == UserInfoShowerMode::Adding && m_userInfo->name.second.empty())
    {
        ui->le_user->setFocus();
        return;
    }
    if(m_userInfo->pic.first)
    {
        QString file = ui->le_file->text();

        //以二进制格式打开图像文件
        FILE *fp = fopen(file.toStdString().c_str(), "rb");
        if(fp == 0)
        {
            QWidget* win = qApp->activeWindow();

            return;
        }

        //获取文件大小
        fseek(fp, 0, SEEK_END);
        int flen = ftell(fp);

        //读取数据
        fseek(fp, 0, SEEK_SET);
        char *data = (char*)malloc(flen + 1);
        int size = fread(data, 1, flen, fp);
        m_userInfo->pic.second.clear();
        m_userInfo->pic.second.append(data, size);
        free(data);
    }

    UserInfoManager um;

    switch(m_mode )
    {
    case UserInfoShowerMode::Adding:
    {
        um.addUser(this->getUserInfo());
        emit this->userAdded(QString::fromStdString(m_userInfo->name.second));
        m_mode = UserInfoShowerMode::Editing;//增加用户后,应为修改模式
    }
        break;
    case UserInfoShowerMode::Editing:
        um.updateUserInfo(this->m_userInfo);
        break;
    default:
        break;

    }


    ui->bt_upload->setDisabled(true);
}

void UserInfoShower::onUserNameChanged(const QString &name)
{
    m_userInfo->name.second = name.toStdString();

    emit nameChanged(name);
}
