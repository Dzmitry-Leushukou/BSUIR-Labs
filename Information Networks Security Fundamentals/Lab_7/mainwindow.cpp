#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_chooseFileButton_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(this,"Choose file for obfustaction");
    qDebug()<<filepath;

    if(filepath=="")
    {
        return;
    }

    ui->filepath->setText(filepath);

    successStatus("File choosed successfuly"),isObfuscated=false;
}

void MainWindow::successStatus(QString status)
{
    ui->status->setText(status);
    ui->status->setStyleSheet("color: green");
}

void MainWindow::errorStatus(QString status)
{
    ui->status->setText(status);
    ui->status->setStyleSheet("color: red");
}

void MainWindow::crypto(bool encrypt)
{
    QFile file(ui->filepath->toPlainText());
    if (!file.open(QIODevice::ReadOnly)) {
        errorStatus("Can`t open this file");
        return;
    }
    QByteArray data = file.readAll();

    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ key[i % key.size()];
    }
    QString encryptedPath=ui->filepath->toPlainText();
    if (encrypt){
        int size =encryptedPath.size();
        if (encryptedPath[size-1]=='c'&&encryptedPath[size-2]=='e'&&encryptedPath[size-3]=='d'&&encryptedPath[size-4]=='.')
        {
            encryptedPath.erase(encryptedPath.begin()+size-4,encryptedPath.end());
        }
        else
            encryptedPath += ".enc";
    }

    else {
        int size =encryptedPath.size();
        if (encryptedPath[size-1]=='c'&&encryptedPath[size-2]=='n'&&encryptedPath[size-3]=='e'&&encryptedPath[size-4]=='.')
        {
            encryptedPath.erase(encryptedPath.begin()+size-4,encryptedPath.end());
        }
         else
            encryptedPath += ".dec";
    }


    QFile encryptedFile(encryptedPath);
    if (!encryptedFile.open(QIODevice::WriteOnly)) {
        errorStatus("Can't create encrypted file");
        return;
    }

    encryptedFile.write(data);
    encryptedFile.close();
    qDebug()<<encryptedPath;
    successStatus("File crypto to: "+ encryptedPath);
}

void MainWindow::on_encryptButton_clicked()
{
    if(ui->filepath->toPlainText()=="")
    {
        errorStatus("Firstly choose file and obfuscate it");
        return;
    }

    try {

        crypto();
    } catch (std::exception e) {
        errorStatus(e.what());
    }

}


void MainWindow::on_decryptButton_clicked()
{
    if(ui->filepath->toPlainText()=="")
    {
        errorStatus("Firstly choose file and obfuscate it");
        return;
    }

    try {
        crypto(false);
    } catch (std::exception e) {
        errorStatus(e.what());
    }
}


void MainWindow::on_obfuscateButton_clicked()
{
    if(ui->status->text()=="")
    {
        errorStatus("Firstly choose file");
        return;
    }
    try {
        obfuscate();
    } catch (std::exception e) {
        errorStatus(e.what());
    }
    isObfuscated=true;
}

void MainWindow::obfuscate()
{
    QString inputPath = ui->filepath->toPlainText();
    if (inputPath.isEmpty()) {
        errorStatus("No file selected");
        return;
    }

    QFile file(inputPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorStatus("Cannot open input file");
        return;
    }
    QString code = QString::fromUtf8(file.readAll());
    file.close();

    rename_identifiers(code);

    QString outputPath = inputPath + ".obf";
    QFile outFile(outputPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorStatus("Cannot create output file");
        return;
    }
    outFile.write(code.toUtf8());
    outFile.close();

    successStatus("Obfuscated file saved to: " + outputPath);
}


void MainWindow::rename_identifiers(QString& code)
{

    QString new_code="";
    std::unordered_map<QString,QString>change;
    enum state {DEFAULT,LINE_COMMENT,COMMENT_BLOCK,STRING,CHAR};
    state cur_state=DEFAULT;
    QString cur="";
    long long index=-1, skip_until=0;
    for(auto& i:code)
    {

        qDebug()<<i<<" "<<cur_state;
        index++;

        if(index<skip_until)
        {
            cur+=i;
            continue;
        }
        if(i=='"')
        {
            if(cur_state==STRING)
            {
                cur_state=DEFAULT;
            }
            else if(cur_state==DEFAULT)
                cur_state=STRING;
        }


        if(i=='\'')
        {
            if(cur_state==CHAR)
            {
                cur_state=DEFAULT;
            }
            else if(cur_state==DEFAULT)
                cur_state=CHAR;
        }


        if(cur.size()>0&&i=='/'&&cur[cur.size()-1]=='/')
        {
            if(cur_state==DEFAULT)
                cur_state=LINE_COMMENT;
        }

        if(cur.size()>0&&i=='*'&&cur[cur.size()-1]=='/')
        {
            if(cur_state==DEFAULT)
                cur_state=COMMENT_BLOCK;
        }


        if(cur.size()>0&&i=='/'&&cur[cur.size()-1]=='*')
        {
            if(cur_state==COMMENT_BLOCK)
                cur_state=DEFAULT;
        }

        if (cur_state==DEFAULT)
        {
            if(i.isLetter()||i=='_')
            {
                unsigned long long start = index;
                QString name;
                int end = start;
                while (end < code.size() && (code[end].isLetterOrNumber() || code[end] == '_')) {
                    name += code[end];
                    end++;
                }
                qDebug()<<"Found name:"<<name;
                if(keywords.find(name.toStdString()) == keywords.end())
                {
                    if(change.count(name)==0)
                    {
                        change[name]="x"+QString::fromStdString(std::to_string(change.size()));

                    }
                    qDebug()<<"It`s not keyword. Change: "<<change[name];
                    new_code+=change[name];
                    skip_until=end;
                    continue;
                }
                qDebug()<<"Is`s keyword";
                new_code+=name;
                skip_until=end;
                continue;

            }
        }

        cur+=i;
        new_code+=i;
        if(i=='\n')
        {
            if(cur_state==LINE_COMMENT)
                cur_state=DEFAULT;

            cur="";
        }

    }

    code=new_code;
}