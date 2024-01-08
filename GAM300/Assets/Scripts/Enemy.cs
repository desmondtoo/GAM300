﻿using System.Collections;
using System.Collections.Generic;
using BeanFactory;
using GlmSharp;
using System;
using System.Threading;

public class Enemy : Script
{
    public float moveSpeed = 2f;
    public float chaseDistance = 5f;
    public float attackDistance = 3f;
    public float attackCooldown = 0f;
    public Transform player;
    public int count;

    public float maxHealth = 3f;
    public float currentHealth;
    // HealthBar
    public Transform hpBar;
    public bool inRange = false;
    public bool back = false;

    public Transform meleeEnemyPos;
    public float RotationSpeed = 6f;


    AnimationStateMachine animationManager;
    public Animator animator;
    public int state;//Example 1 is walk, 2 is attack, 3 is idle etc.
    public bool startDeathAnimationCountdown = false;
    float animationTimer = 3.18f;
    public float currentAnimationTimer;
    private Coroutine damagedCoroutine = null;

    public float testingValue = 0f;



    void Start()
    {
        currentHealth = maxHealth;
        state = 0;//start with idle state
        InitAnimStates();
    }


    void Update()
    {
        if (player == null)
        {
            //SetState("Idle", true);
            return;
        }

        ////follow target
        //vec3 direction = (player.localPosition - transform.localPosition).Normalized;
        //direction.y = 0;
        ////double angle = Math.Atan2(direction.x, direction.z);
        ////vec3.Distance(player.localPosition, this.transform.localPosition);

        //if (vec3.Distance(player.localPosition, transform.localPosition) <= chaseDistance)
        //{
        //    SetState("Run", true);
        //    float angle = (float)Math.Atan2(direction.x,direction.z);
        //    transform.localRotation = new vec3(0,angle,0);
        //    GetComponent<Rigidbody>().linearVelocity = direction * moveSpeed;
        //}

        //NOTE: testing state, remove this later
        if (Input.GetKey(KeyCode.K))
        {
            Console.WriteLine("TestingState");
            //SetState("Run", true);
        }


        vec3 direction = player.localPosition - transform.position;
        direction.y = 0f;
        direction = direction.NormalizedSafe;

        switch (state)
        {
            //idle state
            case 0:
                Console.WriteLine("Idle");
                //idle animation
                SetState("Idle", true);

                //player detection
                if (vec3.Distance(player.localPosition, transform.localPosition) <= chaseDistance)
                {
                    
                    if(animationManager.GetState("Idle").state)
                    {
                        SetState("Idle", false);
                    }
                    //change to chase state
                    state = 1;
                }
                break;
            //chase state
            case 1:
                Console.WriteLine("Chase");
                SetState("Run", true);
                //change to attack state once it has reach it is in range
                if (vec3.Distance(player.localPosition, transform.localPosition) <= attackDistance)
                {
                    state = 2;
                    attackCooldown = 0f;
                }
                //return to its starting position if player is far from its chaseDistance
                if (vec3.Distance(player.localPosition, transform.localPosition) > chaseDistance)
                {
                    if (animationManager.GetState("Run").state)
                    {
                        SetState("Run", false);
                    }
                    //return back to its previous position state
                    state = 0;
                }
                LookAt(direction);
                GetComponent<Rigidbody>().linearVelocity = direction * moveSpeed;
                break;
            //attack state
            case 2:
                Console.WriteLine("Attack");
                //attack animation
                SetState("Attack", true);
                
                LookAt(direction);
                //change to chase state once player has reach out of range
                if (vec3.Distance(player.localPosition, transform.localPosition) > attackDistance)
                {
                    if (animationManager.GetState("Attack").state)
                    {
                        SetState("Attack", false);
                    }
                    state = 1;
                }
                break;
                
        }
    }

    void LookAt(vec3 dir)
    {
        if (dir == vec3.Zero)
            return;
        float angle = (float)Math.Atan2(dir.x, dir.z);
        quat newQuat = glm.FromEulerToQuat(new vec3(0, angle, 0)).Normalized;
        quat oldQuat = glm.FromEulerToQuat(transform.localRotation).Normalized;

        // Interpolate using spherical linear interpolation (slerp)
        quat midQuat = quat.SLerp(oldQuat, newQuat, Time.deltaTime * RotationSpeed);

        vec3 rot = ((vec3)midQuat.EulerAngles);

        if (rot != vec3.NaN)
        {
            bool isNan = false;
            foreach (float val in rot)
            {
                if (float.IsNaN(val))
                {
                    isNan = true;
                    break;
                }
            }
            if (!isNan)
            {
                transform.localRotation = rot;
            }
        }
    }

    // Start is called before the first frame update
    void InitAnimStates()
    {
        animationManager = new AnimationStateMachine(animator);
        currentAnimationTimer = animationTimer;

        //Highest Precedence
        AnimationState death = animationManager.GetState("Death");
        AnimationState stun = animationManager.GetState("Stun");
        AnimationState attack = animationManager.GetState("Attack");
        AnimationState walk = animationManager.GetState("Walk");
        AnimationState run = animationManager.GetState("Run");
        AnimationState idle = animationManager.GetState("Idle");
        //Lowest Precedence

        stun.SetConditionals(false, death);
        attack.SetConditionals(false, death, stun);
        attack.speed = 1.5f;
        walk.SetConditionals(true, walk);
        walk.SetConditionals(false, attack, death, stun);
        walk.loop = true;
        run.SetConditionals(false, attack, death, stun);
        run.loop = true;
    }

    bool GetState(string stateName)
    {
        return animationManager.GetState(stateName).state;
    }
    void SetState(string stateName, bool value)
    {
        animationManager.GetState(stateName).state = value;
    }

    void TakeDamage(int amount)
    {
        currentHealth -= amount;
        hpBar.localScale.x = currentHealth / maxHealth;
    }

    void Exit()
    {

    }

    void OnTriggerEnter(PhysicsComponent other)
    {
        //check if the rigidbody belongs to a game object called PlayerWeaponCollider
        if (GetTag(other) == "PlayerAttack")
        {
            Transform otherT = other.gameObject.GetComponent<Transform>();
            vec3 dir = otherT.back;
            dir = dir.NormalizedSafe;
            if (damagedCoroutine != null)
            {
                StopCoroutine(damagedCoroutine);
            }
            //damagedCoroutine = StartCoroutine(Damaged(.5f, dir * 5));
            TakeDamage(1);
        }
    }

    void OnCollisionEnter(PhysicsComponent component)
    {
        //if (GetTag(component) == "PlayerCollider")
        //{
        //    Console.WriteLine("I have been attacked!");
        //    currentHealth -= 1;
        //    if (currentHealth <= 0)
        //        Destroy(gameObject);
        //}
    }
}

