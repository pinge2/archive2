import math
import torch
import torch.nn as nn
import torch.nn.functional as F


def OrtInit(layer, gain=math.sqrt(2)):
    if isinstance(layer, nn.Linear):
        nn.init.orthogonal_(layer.weight, gain=gain)
        if layer.bias is not None:
            nn.init.constant_(layer.bias, 0.0)
    return layer


class A2C:
    def __init__(self, obs_dim: int = 128, action_dim: int = 8, gamma: float = 0.99):
        self.obs_dim = obs_dim
        self.action_dim = action_dim
        
        self.critic = nn.Sequential(
            OrtInit(nn.Linear(obs_dim, 512)),
            nn.LayerNorm(512),
            nn.ReLU(),
            
            OrtInit(nn.Linear(512, 256)),
            nn.LayerNorm(256),
            nn.ReLU(),
            
            OrtInit(nn.Linear(256, 64)),
            nn.LayerNorm(64),
            nn.ReLU(),
            
            OrtInit(nn.Linear(64, 1), gain=0.01)
        )
        
        self.actor = nn.Sequential(
            OrtInit(nn.Linear(obs_dim, 512)),
            nn.LayerNorm(512),
            nn.ReLU(),
            
            OrtInit(nn.Linear(512, 512)),
            nn.LayerNorm(512),
            nn.ReLU(),
            
            OrtInit(nn.Linear(512, 256)),
            nn.LayerNorm(256),
            nn.ReLU(),
            
            OrtInit(nn.Linear(256, action_dim), gain=0.01),
        )
        
        self.log_std = nn.Parameter(torch.zeros(action_dim))
        
        self.opt_actor = torch.optim.Adam(
            list(self.actor.parameters()) + [self.log_std], 
            lr=0.001
        )
        self.opt_critic = torch.optim.Adam(self.critic.parameters(), lr=0.001)
        
        self.gamma = gamma
        
        self.states = []
        self.actions = []
        self.rewards = []
        self.dones = []
    
    
    def act(self, state: torch.Tensor):
        with torch.no_grad():
            mean = self.actor(state)
            std = torch.exp(self.log_std)
            action = mean + std * torch.randn_like(mean)
        
        return action
    
    
    def store(self, state: torch.Tensor, action: torch.Tensor, reward: float, done: bool):
        self.states.append(state.cpu())
        self.actions.append(action.cpu())
        self.rewards.append(reward)
        self.dones.append(1.0 if done else 0.0)
    
    
    def learn(self):
        states = torch.stack(self.states)
        actions = torch.stack(self.actions)
        rewards = torch.tensor(self.rewards, dtype=torch.float32)
        dones = torch.tensor(self.dones, dtype=torch.float32)
        
        returns = []
        R = 0
        
        for idx in range(len(rewards)-1, -1, -1):
            R = rewards[idx].item() + self.gamma * R * (1 - dones[idx])
            
            returns.insert(0, R)
        
        returns = torch.tensor(returns, dtype=torch.float32).unsqueeze(1)
        returns = (returns - returns.mean()) / (returns.std() + 1e-8)

        values = self.critic(states)
        loss_critic = F.mse_loss(values, returns)
        
        advantage = (returns - values).detach()
        advantage = (advantage - advantage.mean()) / (advantage.std() + 1e-8)
        
        mean = self.actor(states)
        std = torch.exp(self.log_std)
        
        log_prob = -0.5 * (
            ((actions - mean) ** 2) / (std ** 2) 
            + 2 * torch.log(std) 
            + math.log(2 * math.pi)
        ).sum(dim=1, keepdim=True)
        
        entropy = 0.5 * (1 + torch.log(2 * math.pi * std ** 2)).sum()
        
        loss_actor = -(log_prob * advantage).mean() - 0.01 * entropy
            
        self.opt_actor.zero_grad()
        loss_actor.backward()
        torch.nn.utils.clip_grad_norm_(self.actor.parameters(), 0.5)
        self.opt_actor.step()
            
        self.opt_critic.zero_grad()
        loss_critic.backward()
        torch.nn.utils.clip_grad_norm_(self.critic.parameters(), 0.5)
        self.opt_critic.step()
            
        self.states = []
        self.actions = []
        self.rewards = []
        self.dones = []
            
        return loss_actor.item(), loss_critic.item()
   
    
    def save(self, path):
        torch.save({
            'actor': self.actor.state_dict(),
            'log_std': self.log_std,
            'critic': self.critic.state_dict(),
            'opt_actor': self.opt_actor.state_dict(),
            'opt_critic': self.opt_critic.state_dict()
        }, path)

    def load(self, path):
        checkpoint = torch.load(path)
        self.actor.load_state_dict(checkpoint['actor'])
        self.log_std = checkpoint['log_std']
        self.critic.load_state_dict(checkpoint['critic'])
        self.opt_actor.load_state_dict(checkpoint['opt_actor'])
        self.opt_critic.load_state_dict(checkpoint['opt_critic'])