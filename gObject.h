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


#ifndef GOBJECT_H
#define GOBJECT_H

#include <list>
#include <stdio.h>

#define slots
#define signals public
#define emit

namespace Giveda
{
class GObjectPrivate;
class GObject;
template<typename T>
class GSlotAPI;
template<typename GReceiver, typename GRet>
class GSlotCpp;
template<typename T>
class GSignal;

template<typename GRet, typename ...GArgs>
class  GSlotAPI<GRet ( GArgs... ) >
{
public:
    explicit GSlotAPI ( void* slot, GObject* receiver = nullptr )
        :m_receiver ( receiver ),m_slot ( slot ) {}
    GObject* getReceiver()
    {
        return m_receiver;
    }
    virtual void operator() ( GArgs&... args ) = 0;

private:
    void* m_slot;
    GObject* m_receiver;
    friend class GObject;
};

template<typename GReceiver, typename GRet, typename ...GArgs>
class GSlotCpp<GReceiver, GRet ( GArgs... ) > : public GSlotAPI<GRet ( GArgs... ) >
{
public:
    typedef void ( GReceiver::* GSlotPtr ) ( GArgs... );

    GSlotCpp ( GReceiver* receiver, GSlotPtr slot ) : GSlotAPI<GRet ( GArgs... ) > ( ( void* ) &slot, ( GObject* ) receiver ),
        m_slotfunc ( slot ) {}

public:
    void operator() ( GArgs&... args )
    {
        ( ( ( GReceiver* ) GSlotAPI<GRet ( GArgs... ) >::getReceiver() )->*m_slotfunc ) ( args... );
    }

private:
    GSlotPtr m_slotfunc;
    friend class GObject;
};

template<typename GRet, typename... GArgs>
class  GSignal<GRet ( GArgs... ) >
{
public:
    void operator() ( GArgs... args )
    {
        for ( auto iter = _slots.begin(); iter != _slots.end(); ++iter )
            ( **iter ) ( args... );
    }
    ~GSignal()
    {
        for ( auto _slot : _slots )
            delete _slot;
    }
private:
    std::list<GSlotAPI<GRet ( GArgs... ) >* >  _slots;
    friend class GObject;
};

class  GSlot
{
public:
    explicit GSlot ( void* slot, GObject* receiver = nullptr )
        :m_slot ( slot ),
        m_receiver ( receiver ) {}
public:
    bool operator== ( const GSlot& other )
    {
        return ( other.m_slot == m_slot ) && ( other.m_receiver == m_receiver );
    }

private:
    void* m_slot;
    GObject* m_receiver;

    friend class GObject;
    friend class GReceiverFind;
};

class GObject
{
public:
    explicit GObject();
    virtual ~GObject();

    template<class GReceiver, typename ...GArgs>
    static bool  connect ( GObject* sender, GSignal<void ( GArgs... ) >& signal, GReceiver* receiver, void ( GReceiver::* SlotFunc ) ( GArgs... ) );

    template<class GReceiver, typename ...GArgs>
    static bool  disconnect ( GObject* sender, GSignal<void ( GArgs... ) >& signal, GReceiver* receiver, void ( GReceiver::* SlotFunc ) ( GArgs... ) );

private:
    static bool  __connect ( GObject* sender, std::list<GSlot*>* signal, GObject* receiver, void* slot );
    static bool  __disconnect ( GObject* sender, std::list<GSlot*>* signal, GObject* receiver, void* slot );
    void pushSignal ( GObject* sender, std::list<GSlot*>* signal );
    void freeSignal ( GObject* sender, std::list<GSlot*>* signal );
    void pushSlot ( GObject* receiver );
    void freeSlot ( GObject* receiver );
    void release();

private:
    GObjectPrivate* m_private;
};

template<class GReceiver, typename ...GArgs>
bool  GObject::connect ( GObject* sender, GSignal<void ( GArgs... ) >& signal, GReceiver* receiver, void ( GReceiver::* SlotFunc ) ( GArgs... ) )
{
    GSlotCpp<GReceiver, void ( GArgs... ) >* _slotfunc = new GSlotCpp<GReceiver, void ( GArgs... ) > ( receiver, SlotFunc );
    return __connect ( sender, reinterpret_cast<std::list<GSlot*>*> ( & ( signal._slots ) ), ( GObject* ) receiver, ( void* ) _slotfunc );
}

template<class GReceiver, typename ...GArgs>
bool  GObject::disconnect ( GObject* sender, GSignal<void ( GArgs... ) >& signal, GReceiver* receiver, void ( GReceiver::* SlotFunc ) ( GArgs... ) )
{
    return __disconnect ( sender, reinterpret_cast<std::list<GSlot*>*> ( & ( signal._slots ) ), ( GObject* ) receiver, ( void* ) &SlotFunc );
}
}

#endif // GOBJECT_H
