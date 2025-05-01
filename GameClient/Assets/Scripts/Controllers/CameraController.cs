using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraController : MonoBehaviour
{
    public GameObject Target;  // 카메라가 따라다닐 타겟 (캐릭터)

    [SerializeField]
    float _distance = 7.0f;  // 타겟과의 거리
    [SerializeField]
    float _mouseSensitivity = 5.0f;  // 마우스 민감도
    [SerializeField]
    float _srcollSensitivity = 2.0f;  // 스크롤 민감도

    float _minYAngle = 10f;  // 카메라의 최소 y각도 제한
    float _maxYAngle = 80f;  // 카메라의 최대 y각도 제한

    float _minScrollValue = 4.0f;
    float _maxScrollValue = 12.0f;

    Vector2 _moveDelta = new Vector2(0f, 10f);

    //float       _smoothTime = 0.3f;
    //Vector3     _velocity = Vector3.zero;

    public Quaternion PlanarRotation => Quaternion.Euler(0, _moveDelta.x, 0);

    void Update()
    {
        if (Target == null)
            return;

        if (Input.GetMouseButton(1))
        {
            // 마우스 입력을 받아 회전 값을 업데이트
            _moveDelta.x += Input.GetAxis("Mouse X") * _mouseSensitivity;
            _moveDelta.y -= Input.GetAxis("Mouse Y") * _mouseSensitivity;

            // 카메라의 y각도는 제한된 범위 내에서만 움직이도록 설정
            _moveDelta.y = Mathf.Clamp(_moveDelta.y, _minYAngle, _maxYAngle);
        }

    }

    void LateUpdate()
    {
        if (Target == null)
            return;

        Sync();
    }

    public void Sync()
    {
        // 회전 매트릭스를 생성
        Quaternion rotation = Quaternion.Euler(_moveDelta.y, _moveDelta.x, 0);

        float scrollDelta = -Input.GetAxis("Mouse ScrollWheel") * _srcollSensitivity;
        float distance = _distance + scrollDelta;
        _distance = Mathf.Clamp(distance, _minScrollValue, _maxScrollValue);

        // 타겟에서 일정한 거리에 있는 위치를 계산
        Vector3 direction = new Vector3(0, 0, -_distance);
        Vector3 position = Target.transform.position + rotation * direction;

        //transform.position = Vector3.SmoothDamp(transform.position, position, ref _velocity, _smoothTime);

        // 카메라 위치와 회전을 설정
        transform.position = position;
        transform.LookAt(Target.transform.position);
    }
}
