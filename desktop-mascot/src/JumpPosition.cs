using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class JumpPosition : MonoBehaviour
{
    public Vector3 newPosition = new (0, -1.4f, -9.0f);

    void Start()
    {
        StartCoroutine(PauseRoutine());
    }

    void Update()
    {
        
    }

    IEnumerator PauseRoutine()
    {
        // 指定時間待機した後、オブジェクトを指定位置へ移動
        yield return new WaitForSeconds(1.1f);
        transform.position = newPosition;
    }
}
