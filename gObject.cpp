/*
 * Copyright (C) 2019  明心  <imleizhang@qq.com>
 * All rights reserved.
 * 
 * This program is an open-source software; and it is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. 
 * This program is not a free software; so you can not redistribute it and/or 
 * modify it without my authorization. If you only need use it for personal
 * study purpose(no redistribution, and without any  commercial behavior), 
 * you should accept and follow the GNU AGPL v3 license, otherwise there
 * will be your's credit and legal risks.  And if you need use it for any 
 * commercial purpose, you should first get commercial authorization from
 * me, otherwise there will be your's credit and legal risks. 
 *
 */

#include <gObject.h>
#include <string>
#include <algorithm>
#include <string.h>

namespace Giveda
{
class SIGNALS
{
public:
    SIGNALS ( GObject* _sender, std::list<GSlot*>* _signal )
        :sender ( _sender ), signal ( _signal ) { }
    bool operator== ( const SIGNALS& r ) const
    {
        return r.sender == sender && r.signal == signal;
    }
public:
    GObject* sender;
    std::list<GSlot*>* signal;
};

class GReceiverFind
{
public:
    GReceiverFind ( GObject* receiver ) :m_receiver ( receiver ) {}
    bool operator() ( GSlot*& other )
    {
        return other->m_receiver == this->m_receiver;
    }
    bool operator() ( GObject* receiver )
    {
        return this->m_receiver == receiver;
    }
private:
    GObject* m_receiver;
};

class GSenderIs
{
public:
    GSenderIs ( GObject* s ) :sender ( s ) {}
    bool operator() ( SIGNALS& other )
    {
        return other.sender == sender;
    }
private:
    GObject* sender;
};

class GObjectPrivate
{
public:
    ~GObjectPrivate()
    {
        m_signals.clear();
        m_senders.clear();
    }

    std::list<SIGNALS>  m_signals;
    std::list<GObject*>  m_senders;
};

GObject::GObject()
    :m_private ( new GObjectPrivate )
{
}

GObject::~GObject()
{
    release();
    delete m_private;
}

void GObject::pushSignal ( GObject* sender, std::list<GSlot*>* signal )
{
    SIGNALS sp ( sender, signal );
    m_private->m_signals.push_back ( sp );
}

void GObject::freeSignal ( GObject* sender, std::list<GSlot*>* signal )
{
    m_private->m_signals.remove ( SIGNALS ( sender, signal ) );
}

void GObject::release()
{
    for ( auto signaIter = m_private->m_signals.begin(); signaIter != m_private->m_signals.end(); signaIter++ )
    {
        signaIter->signal->remove_if ( GReceiverFind ( this ) );
        signaIter->sender->m_private->m_senders.remove_if ( GReceiverFind ( this ) );
    }

    for ( auto senderIter = m_private->m_senders.begin(); senderIter != m_private->m_senders.end(); senderIter++ )
    {
        GObject* receiver = *senderIter;
        receiver->m_private->m_signals.remove_if ( GSenderIs ( this ) );
    }
}

void GObject::pushSlot ( GObject* receiver )
{
    m_private->m_senders.push_front ( receiver );
}

void GObject::freeSlot ( GObject* receiver )
{
    std::list<GObject*>::iterator it = std::find ( m_private->m_senders.begin(), m_private->m_senders.end(), receiver );
    if ( it == m_private->m_senders.end() ) return;
    m_private->m_senders.erase ( it );
}

bool GObject::__connect ( GObject* sender, std::list<GSlot*>* signal, GObject* receiver, void* slot )
{
    if ( sender == 0 || receiver == 0 || signal == 0 || slot == 0 ) return false;

    GSlot* m_slot = ( GSlot* ) slot;

    void* funcptr = 0;
    void* _funcptr = 0;
    memcpy ( &funcptr, m_slot->m_slot, sizeof ( void* ) );
    memcpy ( &_funcptr, funcptr, sizeof ( void* ) );

    for ( auto sig : *signal )
    {
        if ( * ( ( GSlot* ) slot ) == *sig )
        {
            return false;
        }
    }

    signal->push_back ( m_slot );
    sender->pushSlot ( receiver );
    receiver->pushSignal ( sender, signal );

    return true;
}

bool GObject::__disconnect ( GObject* sender, std::list<GSlot*>* signal, GObject* receiver, void* slot )
{
    bool _b = false;

    if ( sender == 0 || receiver == 0 || signal == 0 || slot == 0 ) return _b;

    std::list< GSlotCpp<GObject, void ( void ) >* >* SlotFunctionType = ( std::list<GSlotCpp<GObject, void ( void ) >*>* ) signal;

    void* funcptr = 0;
    memcpy ( &funcptr, slot, sizeof ( void* ) );

    for ( auto iter = signal->begin(); iter != signal->end(); iter++ )
    {
        auto _funcType = SlotFunctionType->begin();
        void* savePtr = 0;
        memcpy ( &savePtr, & ( ( *_funcType )->m_slotfunc ),sizeof ( void* ) );

        if ( savePtr == funcptr )
        {
            auto freePtr = *iter;
            delete ( freePtr );
            signal->erase ( iter );
            break;
        }
        _funcType++;
    }
    sender->freeSlot ( receiver );
    receiver->freeSignal ( sender, signal );
    return _b;
}

}

