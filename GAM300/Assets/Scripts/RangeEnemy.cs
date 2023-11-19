﻿using System.Collections;
using System.Collections.Generic;
using BeanFactory;
using GlmSharp;
using System;

public class RangeEnemy : Script
{
    public float maxHealth = 3f;
    public float currentHealth;
    public GameObject bulletPrefab;

    void Start()
    {
        Instantiate(bulletPrefab,transform.localPosition,transform.localRotation);
        currentHealth = maxHealth;
    }

    void Update()
    {

    }

    void TakeDamage(int amount)
    {
        currentHealth -= amount;
        Console.WriteLine("Hit");

        if(currentHealth <= 0)
        {
            Destroy(this.gameObject);
        }
    }

    void OnTriggerEnter(Collider other)
    {
        Console.WriteLine("Hit");

        //check if the rigidbody belongs to a game object called PlayerWeaponCollider
        if(other.gameObject.name == "PlayerWeaponCollider")
        {
            Console.WriteLine("Hit");
            TakeDamage(1);

        }
    }

}
