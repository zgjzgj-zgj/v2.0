#include <QTableView>
#include <QSqlQueryModel>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QApplication>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>


/**************************************************修改病人信息**********************************/
class PatientModel :  public QSqlQueryModel
{
public:
    PatientModel()
    {
        this->setQuery("SELECT * FROM patient");
    }

   Qt:: ItemFlags flags(const QModelIndex &index) const override
   {
       Qt::ItemFlags flags = QSqlQueryModel::flags(index);
       if(index.column() == 1 || index.column() == 2)
       flags |=Qt::ItemIsEditable;
       return flags;
   }

  bool setName(int id,QString name)//修改姓名
  {
      bool ok;
        QSqlQuery query;
        query.prepare("UPDATE patient SET name = ? WHERE id = ?");
        query.addBindValue(name);
        query.addBindValue(id);
        ok = query.exec();
        if(!ok)
        {
            qDebug()<<query.lastError();
        }
        return ok;

  }

  bool setSex(int id,QString sex)//修改性别
  {
      bool ok;
        QSqlQuery query;
        query.prepare("UPDATE patient SET sex = ? WHERE id = ?");
        query.addBindValue(sex);
        query.addBindValue(id);
        ok = query.exec();
        if(!ok)
        {
            qDebug()<<query.lastError();
        }
        return ok;

  }

   bool setData(const QModelIndex &index,const QVariant &value, int /* role */ ) override
   {

        if(index.column() < 1 || index.column() > 2)
            return false;


        //获取当前行，列
        QModelIndex primaryKeyIndex = QSqlQueryModel::index(index.row(),0);

        int id = this->data(primaryKeyIndex).toInt();

        bool ok =false;
        if(index.column() == 1)
        {
            //todo:更新姓名
            qDebug()<<"更新姓名";
            ok = setName(id,value.toString());
        }else if(index.column() == 2)
        {
            //todo:更新性别
            qDebug()<<"更新性别";
            ok = setSex(id,value.toString());
       }else
        {
         qDebug()<<"错误";
        }

        if(ok)
            this->setQuery("SELECT * FROM patient");
        return ok;
   }
};

/**************************************************登录*************************************/
int main(int argc, char *argv[])

{
    QApplication a(argc, argv);
   QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
   //服务器地址
   db.setHostName("localhost");
   //数据库
   db.setDatabaseName("finalobject");
   db.setUserName("doctor");
   db.setPassword("123456");
   //
   bool openOK = db.open();

   if(openOK)
       qDebug()<<"建立连接成功";
   else
   {
       qDebug()<<"建立连接失败";
   }
   //
   if(openOK)
   {
/*1.登录数据库，进行连接
 * **********************************************************************************
 * 以上工作段，设备端都要有
 * **/

       QSqlQuery query(db);


       QString userid = "doctor1";
        query.prepare("SELECT name FROM doctor WHERE uid = :id");
        query.bindValue(":id",userid);
        //

        query.exec();
       qDebug()<<query.size();
       while(query.next())
       {
           qDebug()<<query.value("name")<<query.value("mobile");
           QString name  = query.value("name").toString();
           qlonglong mobile = query.value("mobile").toULongLong();
           qDebug()<<name<<mobile;
       }
       if(query.size()<0)
       {
           qDebug()<<query.lastError();
       }

       bool queryOK;

      //工作站从数据库获取数据，给后端
     ;

      query.prepare("SELECT *FROM value WHERE name = :name and dev_id = :dev_id");
      query.bindValue(":start","2020-7-24 19:00:01");
      query.bindValue(":end","2020-7-24 19:30:01");

      queryOK = query.exec();
      if(queryOK)
      {
         while(query.next())
         {
             // QByteArray waveData = query.value("value").toByteArray();
              //todo
              //画波形

              //qDebug()<<waveData;
         }
         qDebug("得到波形数据");

      }else
      {
              qDebug()<<"读波形错误"<<query.lastError();
      }


/*/*
 * 以上代码运行于工作站
 * ***********************************************************************************************************
 * 以下代码运行于设备端
 * ****/

      //获取设备新信息是否在设备中
      query.prepare("SELECT *from device "
                      "WHERE serial = :serial" );
      query.bindValue(":serial","DEV-007");

      int dev_id;
      if(query.exec())
      {
          if(query.size() > 0)
          {
              query.next();
              qDebug()<<"设备已存在";
              dev_id = query.value("dev_id").toInt();
              qDebug()<<"当前设备编号"<< dev_id;
          }else
          {
          query.prepare("INSERT INTO device (serial) VALUES (:serial)");
          query.bindValue(":serial","DEV-007");

          }

     }else
      {
          qDebug()<<"查看设备错误";
      }



      //模拟终端设备，上传数据波形
      //方法二
      query.prepare("INSERT INTO patient (id, name, sex) VALUES (:pid, :pname, :psex )");


      QString pname = "aaa";//***************接受前端的病人名字，从这进入数据库
      query.bindValue(":panme",pname);

     QString id = "doctor1";//******************接受来自前端的id
     query.bindValue(":pid",id);

     QString sex = "男";//*********************接受来自前端的sex
     query.bindValue(":psex",sex);

      query.prepare("INSERT INTO value (value, time, dev_id, name) VALUES (:array, :time, :dev_id, :name )");

      short value[3] = {2048,2049,2030};//*************************接受来自前端的所有波形数据
      QByteArray waves2((char *)value,sizeof(value));

      query.bindValue(":array",waves2);


      //绑定时间
      query.bindValue(":time", QDateTime::currentDateTime());

      //绑定设备id
      query.bindValue(":dev_id",dev_id);//**************************接受来自前端的编号

      //绑定患者姓名
      query.bindValue(":name",pname);//*****************************接受来自前端的患者姓名

      //执行sql语句
     queryOK = query.exec();
     if(!queryOK)
     {
         qDebug()<<"数据写入失败"<<query.lastError();
     }else
     {
         qDebug()<<"数据写入成功";
     }


     //每15秒更新一次字段，判定离线条件位refresh中时间与当前时间差20秒
     query.prepare("UPDATE `finalobject`.`device` SET refresh = NOW() WHERE dev_id = :dev_id");
     query.bindValue(":dev_id",dev_id);
     queryOK = query.exec();
     if(!queryOK)
    {
        qDebug()<<"更新设备状态错误";
     }
   }

   /***下面是工作端,信息显示
    * *******************************************************************************
*/

//显示病人信息
   //创建表格对象
   QTableView *patientview = new QTableView();

   //创建建模对象
   PatientModel patientModel;

  // patientModel.setQuery("SELECT * FROM patient");
   patientview->setModel(&patientModel);
   patientview->show();


   //显示设备列表
   //创建表格对象
   QTableView *view = new QTableView();

   //创建建模对象
   QSqlQueryModel model;

   model.setQuery("SELECT dev_id, serial, now()-refresh < 20 As Oline FROM device");

   view->setModel(&model);
   view->show();



    //显示综合信息
       //创建表格对象
       QTableView *devicepatientview = new QTableView();

       //创建建模对象
       QSqlQueryModel devicepatientModel;

       devicepatientModel.setQuery("SELECT * FROM patient, device, device_patient");
       devicepatientview->setModel(&devicepatientModel);
       devicepatientview->show();
return a.exec();
}
