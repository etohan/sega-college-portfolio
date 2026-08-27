using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class JumpPosition : MonoBehaviour
{
    public Vector3 newPosition = new (0, -1.4f, -9.0f);

    void Start()
    {
        StartCoroutine(PauseRoutine());
        // オブジェクトの位置を新しい位置に設定
        
    }

    void Update()
    {
        
    }

    IEnumerator PauseRoutine()
    {
        // 動作を止める前の処理
        Debug.Log("Pause started");

        // 指定した秒数待機
        yield return new WaitForSeconds(1.1f);

        transform.position = newPosition;
        // 動作を再開する処理
        Debug.Log("Pause ended");
    }
}
