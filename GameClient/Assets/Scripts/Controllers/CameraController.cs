using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class CameraController : MonoBehaviour
{
    public GameObject   Target;  // ī�޶� ����ٴ� Ÿ�� (ĳ����)

    [SerializeField]
    float        _distance = 7.0f;  // Ÿ�ٰ��� �Ÿ�
    [SerializeField]
    float        _mouseSensitivity = 5.0f;  // ���콺 �ΰ���
    
    float        _minYAngle = 10f;  // ī�޶��� �ּ� y���� ����
    float        _maxYAngle = 80f;  // ī�޶��� �ִ� y���� ����

    Vector2     _moveDelta = new Vector2(0f, 10f);

    //float       _smoothTime = 0.3f;
    //Vector3     _velocity = Vector3.zero;

    void Update()
    {
        if (Target == null)
            return;

        if (Input.GetMouseButton(1))
        {
            // ���콺 �Է��� �޾� ȸ�� ���� ������Ʈ
            _moveDelta.x += Input.GetAxis("Mouse X") * _mouseSensitivity;
            _moveDelta.y -= Input.GetAxis("Mouse Y") * _mouseSensitivity;

            // ī�޶��� y������ ���ѵ� ���� �������� �����̵��� ����
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
        // ȸ�� ��Ʈ������ ����
        Quaternion rotation = Quaternion.Euler(_moveDelta.y, _moveDelta.x, 0);

        // Ÿ�ٿ��� ������ �Ÿ��� �ִ� ��ġ�� ���
        Vector3 direction = new Vector3(0, 0, -_distance);
        Vector3 position = Target.transform.position + rotation * direction;

        //transform.position = Vector3.SmoothDamp(transform.position, position, ref _velocity, _smoothTime);

        // ī�޶� ��ġ�� ȸ���� ����
        transform.position = position;
        transform.LookAt(Target.transform.position);
    }
}
