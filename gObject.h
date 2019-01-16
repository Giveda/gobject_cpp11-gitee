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

using namespace std;

class GObjectPrivate;
class GObject;

#define slots

enum E_SLOT_TYPE
{
    C_SLOT_TYPE,
    CPP_SLOT_TYPE,
};

template<typename ...Args>
class  GSlotAPI
{
public:
    E_SLOT_TYPE type() const
    {
        return m_type;
    }

public:
    explicit GSlotAPI(void* slot, GObject* receiver=NULL, E_SLOT_TYPE t=C_SLOT_TYPE )
    :m_receiver(receiver),
    m_slot(slot),
    m_type(t)
    {
    }
    virtual ~GSlotAPI() {}
    virtual void operator() ( Args&... args ) = 0;
    
private:
    GSlotAPI &operator= ( const GSlotAPI & ) = delete;
    
public:
    bool operator==(const GSlotAPI& other)
    {
        return other.m_type == m_type 
        && other.m_slot == m_slot 
        && other.m_receiver == m_receiver ;
    }
    
public:
    GObject* m_receiver;
protected:
    void* m_slot;
    
private:
    E_SLOT_TYPE m_type;
};

template<typename ...Args>
class GSlotC : public GSlotAPI<Args...>
{
private:
    typedef void ( *SlotFuncType ) ( Args... );

public:
    GSlotC ( SlotFuncType slot )
    : GSlotAPI<Args...>( (void*)slot )
    {
    }
    virtual ~GSlotC() {}

public:
    void operator() ( Args&... args )
    {
        ( (SlotFuncType)GSlotAPI<Args...>::m_slot) ( args... );
    }
};

template<typename Receiver, typename ...Args>
class GSlotCpp : public GSlotAPI<Args...>
{
public:
    typedef void ( Receiver::*SlotFuncType ) ( Args... );

    GSlotCpp ( Receiver* r, SlotFuncType slot )
    : GSlotAPI<Args...>( (void*)slot, (GObject*)r, CPP_SLOT_TYPE),
    m_class_slot(slot)
    {
    }

    virtual ~GSlotCpp() {}

public:
    void operator() ( Args&... args )
    {
        ( ( (Receiver*)GSlotAPI<Args...>::m_receiver)->*m_class_slot ) ( args... );
    }
    
private:
    SlotFuncType m_class_slot;
};

/**
 * @class GSignal
 * @brief  GSignal类用来定义信号，所述信号的函数类型为void (*)(Args...)。\n
 * 比如：GSignal<int> intSig;//定义一个函数类型为void intSig(int);
 * 比如：GSignal<int, float> ifSig;//定义一个函数类型为void ifSig(int,  float);
 *
 */
template<typename ...Args>
class GSignal
{
public:
    typedef list<GSlotAPI<Args...>* > SlotLstType;

public:
    /**
     * @brief 发射信号
     * 
     * @param args  参数列表
     * @return void
     */
    void emit ( Args... args )
    {
        for ( auto it = _slotLst.begin(); it != _slotLst.end(); ++it )
        {
            ( * ( *it ) ) ( args... );
        }
    }

    /**
     * @brief 发射信号
     * 
     * @param args  参数列表
     * @return void
     */
    void operator() ( Args... args )
    {
        for ( auto it = _slotLst.begin(); it != _slotLst.end(); ++it )
        {
            ( * ( *it ) ) ( args... );
        }
    }

public:
    SlotLstType  _slotLst;
};


class  GSlot
{
public:
    E_SLOT_TYPE type() const
    {
        return m_type;
    }
    
public:
    explicit GSlot(void* slot, GObject* receiver=NULL, E_SLOT_TYPE t=C_SLOT_TYPE )
    :m_receiver(receiver),
    m_slot(slot),
    m_type(t)
    {
    }
    virtual ~GSlot() {}
    virtual void operator() ( const GSlot& );
    
private:
    GSlot &operator= ( const GSlot & ) = delete;
    
public:
    bool operator==(const GSlot& other)
    {
        return other.m_type == m_type 
        && other.m_slot == m_slot 
        && other.m_receiver == m_receiver ;
    }
    
public:
    GObject* m_receiver;
protected:
    void* m_slot;
    
private:
    E_SLOT_TYPE m_type;
};

#define SIGNAL_TYPE(SlotFuncType)  list<GSlot*>
#define SIGNAL_POINTER(SlotFuncType)  list<GSlot*>*
#define SIGNAL_TYPE_ITERATOR(SlotFuncType)  list<GSlot*>::iterator

#define signals public

class  GObject
{
signals:
    GSignal<GObject*> sigDestroyed;

private:
    GObjectPrivate *m_priv;

public:
    explicit GObject ( GObject *parent=NULL,  const char *name=NULL );
    GObject ( const GObject & src );
    GObject & operator= ( const GObject & src );
    virtual ~GObject();

    /**
     * @brief 将信号和槽建立连接。\n
     * Receiver代表接收者的类型
     * Args是槽函数/信号的参数列表。
     *
     * @param sender 指向发射者的指针
     * @param signal 指向信号的引用。
     * @param receiver 指向接收者的指针
     * @param slot 指向槽函数的指针
     *
     * @return 0代表成功；非0代表失败
     */
    template<class Receiver, typename ...Args>
    static int  connect ( GObject* sender, GSignal<Args...>& signal, Receiver* receiver, void ( Receiver::*SlotFunc ) ( Args... ) );

    /**
     * @brief 将信号和槽断开连接。\n
     * Receiver代表接收者的类型
     * Args是槽函数/信号的参数列表。
     *
     * @param sender 指向发射者的指针
     * @param signal 指向信号的引用。
     * @param receiver 指向接收者的指针
     * @param slot 指向槽函数的指针
     *
     * @return 0代表成功；非0代表失败
     */
    template<class Receiver, typename ...Args>
    static int  disconnect ( GObject* sender, GSignal<Args...>& signal, Receiver* receiver, void ( Receiver::*SlotFunc ) ( Args... ) );

    const char *name() const;
    GObject *parent() const;


private:
    static int  privConnect(GObject* sender, SIGNAL_POINTER(void*) signal, GObject* receiver, void* slot);
    static int  privDisconnect(GObject* sender, SIGNAL_POINTER(void*) signal, GObject* receiver, void* slot);
    void saveSenderPair(GObject* sender, SIGNAL_POINTER(void*) signal);
    void deleteSenderPair(GObject* sender, SIGNAL_POINTER(void*) signal);
    void destructAsReceiver();
    void destructAsSender();
    void saveReceiver ( GObject* receiver );
    void deleteReceiver ( GObject* receiver );
};

template<class Receiver, typename ...Args>
int  GObject::connect ( GObject* sender, GSignal<Args...>& signal, Receiver* receiver, void ( Receiver::*SlotFunc ) ( Args... ) )
{
    GSlotCpp<Receiver, Args...> *vslot = new GSlotCpp<Receiver, Args...>(receiver, SlotFunc);
    int ret = privConnect(sender, reinterpret_cast<SIGNAL_POINTER(void*)>(&(signal._slotLst)), (GObject*)receiver, (void*)vslot);
    if(0 != ret)
    {
        delete vslot;
    }
    return ret;
}

template<class Receiver, typename ...Args>
int  GObject::disconnect ( GObject* sender, GSignal<Args...>& signal, Receiver* receiver, void ( Receiver::*SlotFunc ) ( Args... ) )
{
    int ret = privDisconnect(sender, reinterpret_cast<SIGNAL_POINTER(void*)>(&(signal._slotLst)), (GObject*)receiver, (void*)SlotFunc);
    return ret;
}

#endif // GOBJECT_H
