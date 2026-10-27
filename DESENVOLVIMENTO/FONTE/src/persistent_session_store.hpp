#pragma once
// Crash recovery local to this Windows user; never connects to a machine or
// changes an original .alaser file. Paths are NOT written to exported telemetry.
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
namespace alaser {
struct PersistentSessionStore {
 static QString folder(){
  QString root=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  if(root.isEmpty())return {};
  return root+"/alencar_recovery_v1";
 }
 static QString snapshotPath(){return folder()+"/last_unsaved.alaser";}
 static QString manifestPath(){return folder()+"/last_unsaved.json";}
 static bool hasSnapshot(){return QFile::exists(snapshotPath())&&QFile::exists(manifestPath());}
 static bool writeMarker(const QString& original){
  if(folder().isEmpty()||!QDir().mkpath(folder()))return false;
  QSaveFile marker(manifestPath());
  if(!marker.open(QIODevice::WriteOnly))return false;
  const QByteArray data=QJsonDocument(QJsonObject{
   {"schema","alencar-recovery-1"},
   {"created_utc",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
   {"original_project",original},
   {"restore_requires_consent",true},
   {"never_auto_connect",true}
  }).toJson(QJsonDocument::Indented);
  return marker.write(data)==data.size()&&marker.commit();
 }
 static void discard(){QFile::remove(manifestPath());QFile::remove(snapshotPath());}
 static void recordErrorCode(const QString& code){
  // Bounded, code-only breadcrumbs; not a raw machine message or any IP/path.
  if(code.size()>70||code.isEmpty()||folder().isEmpty())return;
  for(const QChar c:code)if(!c.isLetterOrNumber()&&c!='_')return;
  if(!QDir().mkpath(folder()))return;
  QFile previous(folder()+"/error_codes.txt");
  QByteArray contents;
  if(previous.open(QIODevice::ReadOnly))contents=previous.readAll().right(6000);
  contents+=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs).toUtf8()+" "+code.toUtf8()+"\n";
  QSaveFile destination(folder()+"/error_codes.txt");
  if(destination.open(QIODevice::WriteOnly)){
   if(destination.write(contents.right(6000))==contents.right(6000).size())destination.commit();
   else destination.cancelWriting();
  }
 }
};
}
