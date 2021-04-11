#ifndef RESOURCE_POOL_H
#define RESOURCE_POOL_H

template <class T, u32 poolSize>
struct ResourcePool
{
private:
    T objs[poolSize];
    u32 handles[poolSize];
    u32 count;

    T *objsToDelete[poolSize];
    void (*deleteCallbacks[poolSize])(T*,u32);
    u32 objsToDeleteCount;
public:
    ResourcePool()
    {
        count = 0;
        objsToDeleteCount = 0;

        for (u32 i = 0; i < poolSize; i++)
        {
            handles[i] = i;
        }
    }

    T *create()
    {
        if (count >= poolSize)
            return nullptr;

        u32 handle = handles[count++];
        return &objs[handle];
    }

    T *create(s32 *outHandle)
    {
        if (count >= poolSize)
        {
            *outHandle = -1;
            return nullptr;
        }

        u32 handle = handles[count++];
        *outHandle = handle;
        return &objs[handle];
    }

    T &operator[](const u32 handle)
    {
        return objs[handle];
    }

    s32 get_handle(u32 i)
    {
        if (i >= count)
            return -1;

        return handles[i];
    }

    u32 get_count() const
    {
        return count;
    }

    void mark_for_destruction(T *obj, void (*callback)(T*,u32) = nullptr)
    {
        u32 index = objsToDeleteCount++;
        objsToDelete[index] = obj;
        deleteCallbacks[index] = callback;
    }

    void destroy_obj(T *obj, void (*callback)(T*,u32) = nullptr)
    {
        //find it
        for (int i = 0; i < count; i++)
        {
            const u32 handle = handles[i];
            T* found = &objs[handle];
            if (found != obj)
                continue;

            count--;
            handles[i] = handles[count];
            handles[count] = handle;

            if (callback != nullptr)
                callback(obj,handle);

            return;
        }
    }

    void destroy_objs()
    {
        for(int i = 0; i < objsToDeleteCount; i++)
        {
            destroy_obj(objsToDelete[i], deleteCallbacks[i]);
        }
        objsToDeleteCount = 0;
    }
};

#endif // RESOURCE_POOL_H
