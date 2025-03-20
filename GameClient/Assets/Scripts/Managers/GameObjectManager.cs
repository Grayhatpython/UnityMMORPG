using Google.Protobuf.Protocol;
using Protocol;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameObjectManager
{
    Dictionary<uint, GameObject> _objects = new Dictionary<uint, GameObject>();   
    public HeroController Hero { get; set; }

    public void HandleSpawn(ObjectInfo objectInfo, bool isHeroPlayer = false)
    {
        if (Hero != null && Hero.Id == objectInfo.ObjectId)
            return;

        if (_objects.ContainsKey(objectInfo.ObjectId))
            return;

        GameObject go = null;
        if (isHeroPlayer)
        {
            go = Managers.Resource.Instantiate("Hero");
            Hero = go.GetComponent<HeroController>();
            Hero.Id = objectInfo.ObjectId;
            Hero.PosInfo = objectInfo.PositionInfo;
            Hero.DestPosInfo = objectInfo.PositionInfo;

            CameraController cc = Camera.main.GetComponent<CameraController>();
            cc.Target = go;
        }
        else
        {
            go = Managers.Resource.Instantiate("Player");
            PlayerController pc = go.GetComponent<PlayerController>();
            pc.Id = objectInfo.ObjectId;    
            pc.PosInfo = objectInfo.PositionInfo;
            pc.DestPosInfo = objectInfo.PositionInfo;
        }

        _objects.Add(objectInfo.ObjectId, go);
    }

    public void HandleSpawn(S_ENTER_GAME enterGamePacket)
    {
        HandleSpawn(enterGamePacket.Player, true);
    }

    public void HandleSpawn(S_SPAWN spawnPacket)
    {
        foreach(ObjectInfo obj in spawnPacket.Objects)
            HandleSpawn(obj);
    }

    public void HandleDespawn(S_DESPAWN despawnPacket)
    {
        foreach(uint id in despawnPacket.ObjectIds)
            HandleDespawn(id);  
    }

    public void HandleDespawn(uint objectId)
    {
        if (_objects.ContainsKey(objectId) == false)
            return;

        GameObject go = null;
        _objects.TryGetValue(objectId, out go);

        if (go == null)
            return;

        _objects.Remove(objectId);
        Managers.Resource.Destroy(go);
    }

    public void HandleMove(S_MOVE movePacket)
    {
        uint objectId = movePacket.PositionInfo.ObjectId;
        if (_objects.ContainsKey(objectId) == false)
            return;

        if (Hero.Id == objectId)
            return;

        GameObject go = null;
        _objects.TryGetValue(objectId, out go);

        if (go == null)
            return;

        //  TODO
        PlayerController pc = go.GetComponent<PlayerController>();
        pc.DestPosInfo = movePacket.PositionInfo;
        pc.State = movePacket.PositionInfo.State;
    }

    public void Clear()
    {
        foreach(GameObject obj in _objects.Values)
            Managers.Resource.Destroy(obj);

        _objects.Clear();
    }
}
