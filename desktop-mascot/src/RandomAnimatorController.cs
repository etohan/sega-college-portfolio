using UnityEngine;
using System.Collections;

public class RandomAnimatorController : MonoBehaviour
{
    public Animator animator;

    [System.Serializable]
    public class AnimData
    {
        public string stateName;
        public float minInterval = 3f;
        public float maxInterval = 6f;
    }

    public AnimData[] animations;

    private string idleState = "HumanF@Idle01";
    private int lastIndex = -1;

    void Start()
    {
        StartCoroutine(RandomAnimationLoop());
    }

    IEnumerator RandomAnimationLoop()
    {
        while (true)
        {
            // 待機（Idle状態）
            yield return new WaitForSeconds(Random.Range(6f, 10f));

            // ランダム選択（同じの連続防止）
            int index;
            do
            {
                index = Random.Range(0, animations.Length);
            }
            while (index == lastIndex && animations.Length > 1);

            lastIndex = index;

            var anim = animations[index];

            // 再生
            animator.CrossFade(anim.stateName, 0.2f);

            // アニメ終了まで待つ
            yield return StartCoroutine(WaitAnimationEnd(anim.stateName));

            // Idleに戻る
            animator.CrossFade(idleState, 0.2f);
        }
    }

    IEnumerator WaitAnimationEnd(string stateName)
    {
        // ステートに入るまで待つ
        while (!animator.GetCurrentAnimatorStateInfo(0).IsName(stateName))
        {
            yield return null;
        }

        // 再生終了まで待つ
        while (animator.GetCurrentAnimatorStateInfo(0).normalizedTime < 1.0f)
        {
            yield return null;
        }
    }
}
