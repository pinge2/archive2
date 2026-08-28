from ppo import *
from game_env import *



class MyGame(Env):
    def __init__(self, agent: A2C, learn = True, headless = False):
        super().__init__((1000, 800), not headless)
        
        self.learn = learn
        self.bg = (150, 150, 150)
        
        self.addStatic((1000, 800), (0, 800))
        self.addStatic((50, 750), (0, 800))
        self.addStatic((50, 750), (50, 700))
        self.addStatic((950, 750), (950, 700))
        
        self.agent = agent
        
        self.max_episodes = 10000
        self.steps_per_episode = 200
        self.update_freq = 32
        
        self.episode = 0
        self.total_steps = 0
        self.episode_reward = 0.0
        self.best_reward = -float('inf')
        
        self.loss_a = 0.0
        self.loss_c = 0.0
        
        self.actions_every = 6
        self.frame_cnt = 0
        
        self.worm = None
        self.apple = None
        
        self.prev_apple_dist = 0
    
    
    def create_apple(self, pos = (800, 680)):
        if self.apple is not None: self.apple.remove()
        
        self.apple = StaticCircleObject(self, pos, 20, 1, (255, 0, 0, 255))
        self.apple.setparams(1, 0.8, 0.7)
    
    
    def reset(self):
        if self.worm is not None: self.worm.remove()
        
        self.worm = Worm(self, (500, 600), 5, 10000, (0, 0, 255, 0))
        
        self.create_apple((np.random.uniform(100, 900), 
                           np.random.uniform(300, 500)))

        self.prev_apple_dist = self.worm.head().dist(self.apple)
        
        obs = self.worm.getObs(self.apple.pos()) # type: ignore
        
        return obs
    
    
    def compute_reward(self):
        assert self.worm is not None
        assert self.apple is not None
        
        dist = self.worm.head().dist(self.apple)
        
        max_dist = 1000.0
        r_progress = (self.prev_apple_dist - dist) / max_dist
        
        self.prev_apple_dist = dist
        
        r_done = 0.0
        if self.is_done():
            r_done = 1.0
        
        r_high = 1.0
        for seg in self.worm.segments:
            if seg.pos().y < 850: r_high -= 1 / 5
        
        r_energy = 0.0
        for seg in self.worm.segments[:-1]:
            r_energy += (1 - seg.getEnergy()) / 4
        
        r_progress = np.clip(r_progress, 0, 1)
        r_high = np.clip(r_high, 0, 1)
        r_done = np.clip(r_done, 0, 1)
        r_energy = np.clip(r_energy, 0, 1)
        
        reward = 0.5*r_progress - 0.05*r_high + 10*r_done - 0.05*r_energy - 0.0005
        
        return reward
    
    
    def is_done(self):
        return self.worm.head().dist(self.apple) < 30 # type: ignore
    
    
    def step(self, action):
        assert self.worm is not None
        assert self.apple is not None
        
        angles = np.clip(action[:4], -1, 1) * 10
        torques = np.clip(action[4:], 0, 1)
        
        for i in range(len(self.worm.segments)-1):
            self.worm.segments[i].setTorque(angles[i].item(), torques[i].item())
        
        obs = self.worm.getObs(self.apple.pos())
        reward = self.compute_reward()
        done = self.is_done()
        
        return obs, reward, done
    
    
    def onStart(self):
        self.obs = self.reset()
        self.total_steps = 0
        self.episode_reward = 0.0
        
        if not self.is_render: print("Начало обучения...")
    
    
    def onMainloop(self, dt, frame):
        self.setText(frame)
        
        self.frame_cnt += 1
        
        if self.frame_cnt % self.actions_every != 0: return
        
        action = self.agent.act(self.obs)
            
        next_obs, reward, done = self.step(action)
            
        if self.learn:
            self.agent.store(self.obs, action, reward, done)
            
        self.obs = next_obs
        self.episode_reward += reward
        self.total_steps += 1
            
        if self.learn and self.total_steps % self.update_freq == 0:
            self.loss_a, self.loss_c = self.agent.learn()
            
            print(f"\tОбновление: actor_loss={self.loss_a:.4f}, critic_loss={self.loss_c:.4f}")
        
        
        if done or self.total_steps >= self.steps_per_episode:
            self.episode += 1
            avg_reward = self.episode_reward / max(1, self.total_steps)
                
            apple_reached = done
            print(f"Эпизод {self.episode}: reward={self.episode_reward:.2f}, "
                f"шагов={self.total_steps}, avg={avg_reward:.4f}, "
                f"яблоко={'✓' if apple_reached else '✗'}")
                
            if self.episode_reward > self.best_reward:
                self.best_reward = self.episode_reward
                self.agent.save("best.model")
                print(f"  => Новая лучшая модель (reward={self.best_reward:.2f})")
                
            self.obs = self.reset()
            self.episode_reward = 0
            self.total_steps = 0
            self.frame_cnt = 0
                
            if self.episode >= self.max_episodes: self.stop()
    
    
    def setText(self, frame):
        assert self.worm is not None
        
        self.addText(f"Worm pos X: {self.worm.head().pos().x}", (0, 0))
        self.addText(f"Worm pos Y: {self.worm.head().pos().y}", (0, 25))
        self.addText(f"Frame: {frame}", (0, 50))
        self.addText(f"Epoch: {self.episode}", (0, 75))
        
        torques, energies = [], []
        
        for i in self.worm.segments[:-1]:
            torques.append(f"{i.getTorque():.3}")
            energies.append(f"{i.getEnergy():.3}") 
        
        self.addText(f"Torques: {', '.join(torques)}", (0, 100))
        self.addText(f"Energy: {', '.join(energies)}", (0, 125))
        
        self.addText(f"Actor loss: {self.loss_a:.3}", (0, 175))
        self.addText(f"Critic loss: {self.loss_c:.3}", (0, 200))
        self.addText(f"Reward: {float(self.episode_reward):.3}", (0, 225))
        
        
    def onKeyUp(self, key):...
        
    def onKeyDown(self, key):...
        
    def onMouseDown(self, button, pos):...


agent = A2C(27, 8)
agent.load("best.model")

print("ИИ загружен...")

e = MyGame(agent, True, False)

e.mainloop()
e.quit()