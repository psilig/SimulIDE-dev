/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include <QList>

class MemTable;
class eMcu;

class MemData
{
    public:
        MemData();
        ~MemData();

        static bool loadData( QList<int>* toData, bool resize=false, int bits=8 );
        static void saveData( QList<int>* data, int bits=8 );

        static bool loadFile( QList<int>* toData, QString file, bool resize, int bits, eMcu* eMcu=nullptr );
        static bool loadDat( QList<int>* toData, QString file, bool resize );
        static bool loadHex( QList<int>* toData, QString file, bool resize, int bits );
        static bool loadBin( QList<int>* toData, QString file, bool resize, int bits );

        static QString getMem( QList<int>* data );
        static void setMem( QList<int>* data, QString m );

        virtual void showTable( int dataSize=256, int wordBytes=1 );

    protected:
        MemTable* m_memTable;
        static eMcu* m_eMcu;

        static void saveDat( QList<int>* data, int bits );
        static void saveHex( QList<int>* data, int bits ); /// TODO
        static void saveBin( QList<int>* data, int bits );
};
