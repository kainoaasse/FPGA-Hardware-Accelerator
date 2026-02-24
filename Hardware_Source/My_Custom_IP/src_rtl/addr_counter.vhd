----------------------------------------------------------------------------------
-- Company: George Mason
-- Engineer: Kainoa Asse
-- Create Date: 05/03/2024 04:46:13 PM
-- Revised:     02/24/2026 (Documentation and Final Verification)
-- Design Name: Sorting core on FPro System
-- Module Name: addr_counter - Behavioral
-- Target Devices: Basys 3 (Artix-7)
-- Tool Versions: Vivado 2023.x / Vitis 2023.x
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
entity addr_counter is
    Port (clk, en, ld : in std_logic;
          Q : out std_logic_vector(12 downto 0)
          );
end addr_counter;

architecture Behavioral of addr_counter is
    signal count : unsigned(12 downto 0) := (OTHERS => '0');
begin
    process(clk) 
    begin
        if(rising_edge(clk)) then
			if(ld = '1') then
		        count <= (OTHERS => '0'); --Load 0 to counter
			elsif (en = '1') then
			    count <= count + 1;
            end if;
        end if;
    end process;
    Q <= std_logic_vector(count);
    
end Behavioral;
