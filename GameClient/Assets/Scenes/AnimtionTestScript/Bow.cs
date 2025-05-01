using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Bow : MonoBehaviour
{
    [System.Serializable]
    public class BowSetting
    {
        [Header("Arrow Settings")]
        public float arrowCount;
        public GameObject arrowPrefab;
        public Transform arrowPosition;

        [Header("Bow Equip & UnEquiep Settings")]
        public Transform equipPosition;
        public Transform unEquipPosition;

        public Transform equipParent;
        public Transform unEquipParent;

        [Header("Bow String Settings")]
        public Transform bowString;
        public Transform stringInitialPosition;
        public Transform stringHandPullPosition;
        public Transform stringInitialParent;
    }

    [SerializeField]
    public BowSetting _bow;

    [Header("Crosshair Settings")]
    public GameObject _crossHairPrefab;
    GameObject _currentCrossHair;

    bool _canPullString = false;
    bool _canFireArrow = false;

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void EquipBow()
    {
        transform.position = _bow.equipPosition.position;
        transform.rotation = _bow.equipPosition.rotation;
        transform.parent = _bow.equipParent; 
    }

    void UnEquipBow()
    {
        transform.position = _bow.unEquipPosition.position;
        transform.rotation = _bow.unEquipPosition.rotation;
        transform.parent = _bow.unEquipParent;
    }

    public void ShowCrosshair(Vector3 crosshairPosition)
    {
        if (_currentCrossHair == null)
            _currentCrossHair = Instantiate(_crossHairPrefab) as GameObject;

        _currentCrossHair.transform.position = crosshairPosition;
        _currentCrossHair.transform.LookAt(Camera.main.transform);
    }

    public void RemoveCrosshair()
    {
        if(_currentCrossHair)
        {
            Destroy(_currentCrossHair);
        }
    }
}
