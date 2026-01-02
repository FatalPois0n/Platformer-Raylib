#include "shooter.hpp"
#include <algorithm>

Shooter::Shooter()
    : projectileSpeed(800.0f),
      fireCooldownSeconds(5.0f),
      lastShotTime(0.0)
{
}

void Shooter::Shoot(Vector2 origin, bool facingRight)
{
    double now = GetTime();
    if (now - lastShotTime < fireCooldownSeconds) return;
    lastShotTime = now;

    Projectile p;
    p.position = origin;
    p.velocity = { facingRight ? projectileSpeed : -projectileSpeed, 0.0f };
    p.radius = 6.0f;
    p.damage = 20.0f;
    p.lifeSeconds = 2.0f; // auto-despawn after 2 seconds
    p.color = facingRight ? RED : ORANGE;
    p.alive = true;

    projectiles.push_back(p);
}

void Shooter::UpdateProjectiles(const std::vector<Platform>& platforms, const std::vector<Wall>& walls)
{
    float dt = GetFrameTime();
    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();

    for (auto& p : projectiles) {
        if (!p.alive) continue;

        // Integrate motion
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;

        // Lifetime
        p.lifeSeconds -= dt;
        if (p.lifeSeconds <= 0.0f) {
            p.alive = false;
            continue;
        }

        // Despawn when off-screen
        if (p.position.x < -50 || p.position.x > screenW + 50 ||
            p.position.y < -50 || p.position.y > screenH + 50) {
            p.alive = false;
            continue;
        }

        // Simple collision with platforms: destroy on impact with any platform top area
        Rectangle r = p.GetRect();
        for (const auto& platform : platforms) {
            if (CheckCollisionRecs(r, platform.GetRect())) {
                p.alive = false;
                break;
            }
        }
        for (const auto& wall : walls) {
            if (CheckCollisionRecs(r, wall.GetRect())) {
                p.alive = false;
                break;
            }
        }
    }

    // Compact dead projectiles
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& pr){ return !pr.alive; }),
        projectiles.end()
    );
}

void Shooter::DrawProjectiles() const
{
    for (const auto& p : projectiles) {
        if (!p.alive) continue;
        DrawCircleV(p.position, p.radius, p.color);
    }
}

void Shooter::ResolveHits(std::vector<Enemy*>& enemies)
{
    for (auto& p : projectiles) {
        if (!p.alive) continue;
        Rectangle pr = p.GetRect();
        for (auto* e : enemies) {
            if (e == nullptr || e->IsDead()) continue;
            if (CheckCollisionRecs(pr, e->GetRect())) {
                e->TakeDamage(p.damage);
                p.alive = false;
                break;
            }
        }
    }
}
