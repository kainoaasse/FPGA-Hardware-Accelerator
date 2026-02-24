----------------------------------------------------------------------------------
-- Company: George Mason
-- Engineer: Kainoa Asse
-- Create Date: 04/23/2024 11:43:01 AM
-- Revised:     02/24/2026 (Documentation and Final Verification)
-- Design Name: Sorting core on FPro System
-- Module Name: Comparator - Behavioral
-- Target Devices: Basys 3 (Artix-7)
-- Tool Versions: Vivado 2023.x / Vitis 2023.x
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity Comparator is
    Port (A : in std_logic_vector(15 downto 0);
          B : in std_logic_vector(15 downto 0);
          AgtB : out std_logic);
end Comparator;

architecture Behavioral of Comparator is

begin
    --Comparison of Mi(A) and Mj(B)
    process (A, B)
    begin
        if unsigned(A) > unsigned(B) then
            AgtB <= '1'; -- A is greater than B
        else
            AgtB <= '0'; -- A is not greater than B
        end if;
    end process;
    
end Behavioral;
