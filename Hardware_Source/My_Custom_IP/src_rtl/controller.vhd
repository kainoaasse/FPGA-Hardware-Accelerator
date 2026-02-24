----------------------------------------------------------------------------------
-- Company: George Mason
-- Engineer: Kainoa Asse
-- Create Date: 04/24/2024 09:29:29 PM
-- Revised:     02/24/2026 (Documentation and Final Verification)
-- Design Name: Sorting core on FPro System
-- Module Name: controller - Behavioral
-- Target Devices: Basys 3 (Artix-7)
-- Tool Versions: Vivado 2023.x / Vitis 2023.x
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity controller is
    Port (clk, s, reset : in std_logic;
          MigtMj, zi, zj : in std_logic; --signals from datapath
          Wr, Li, Ei, Lj, Ej, Done : out std_logic
          );
end controller;

architecture Behavioral of controller is
    --Define states for ASM chart
    type state_type is (S0, S1, S2, S3, S4);
    signal current_state, next_state : state_type;
    
begin
    process(clk, reset)
    begin
        if (reset = '1') then
            current_state <= S0;
        elsif rising_edge(clk) then
            current_state <= next_state;
        end if;
    end process;
    
    -- FSM Logic for ASM chart
    process(current_state, s, MigtMj, zj, zi)
    begin
        next_state <= current_state;
        Done <= '0';
        Wr <= '0';
        Li <= '0';
        Ei <= '0';
        Lj <= '0';
        Ej <= '0';
        
        case current_state is
            when S0 =>
                Li <= '1';
                if (s = '1') then
                    next_state <= S1;
                else 
                    next_state <= S0;
                end if;
            when S1 =>
                Lj <= '1';
                Ej <= '1'; --added control signal to properly initialize j counter
                next_state <= S2;
            when S2 =>
                next_state <= S3;
            when S3 =>
                if (MigtMj = '1') then
                    Wr <= '1';
                end if;
                if (zj = '0') then
                    Ej <= '1';
                    next_state <= S2;
                else
                    if (zi = '0') then
                        Ei <= '1';
                        next_state <= S1;
                    else
                        next_state <= S4;
                    end if;
                end if;
            when S4 =>
                Done <= '1';
                if (s = '0') then
                    next_state <= S0;
                else
                    next_state <= S4;
                end if;
            end case;
    end process; 
                                   
end Behavioral;
