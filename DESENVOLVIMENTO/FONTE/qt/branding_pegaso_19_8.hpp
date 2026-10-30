#pragma once
// Alencar Laser TESTE 19.8 — identidade PEGASO_V1 opcional, offline.
// Nao altera documentos .alaser, CAM, hardware, G-code ou a logo anterior.
// A arte aprovada reside em DADOS/LOGO/PEGASO_V1; sem ela, manter fallback.
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QStringList>

namespace alaser::branding198 {

inline QStringList resourceRoots(){
 QStringList roots;
 const QStringList bases={QDir::currentPath(),QCoreApplication::applicationDirPath()};
 for(const QString& base:bases){
  QDir dir(base);
  for(int depth=0;depth<=5;++depth){
   roots << dir.absoluteFilePath(QStringLiteral("DADOS/LOGO/PEGASO_V1"));
   if(!dir.cdUp())break;
  }
 }
 roots.removeDuplicates();
 return roots;
}

inline QString findInRoots(const QString& name,const QStringList& roots){
 // Somente nomes conhecidos, nunca caminho arbitrario proveniente do usuario.
 if(name.contains('/')||name.contains('\\')||name=="."||name=="..")return {};
 for(const auto& root:roots){
  const QString path=QDir(root).absoluteFilePath(name);
  if(QFileInfo(path).isFile())return path;
 }
 return {};
}

inline QString assetPath(const QString& name){
 return findInRoots(name,resourceRoots());
}

inline QPixmap approvedHeader(){
 const QString path=assetPath(QStringLiteral("marca_cabecalho_compacta.png"));
 return path.isEmpty()?QPixmap():QPixmap(path);
}

inline QIcon approvedIcon(){
 const QString path=assetPath(QStringLiteral("pegaso_icone_256.png"));
 if(path.isEmpty())return {};
 QIcon icon;
 icon.addFile(path,QSize(256,256));
 // Recurso grafico apenas, sem substituir o executavel nem arquivos originais.
 return icon;
}

inline QPixmap approvedSplash(){
 const QString path=assetPath(QStringLiteral("abertura_pegaso.png"));
 return path.isEmpty()?QPixmap():QPixmap(path);
}

} // namespace alaser::branding198
