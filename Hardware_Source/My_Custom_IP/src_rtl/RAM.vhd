----------------------------------------------------------------------------------
-- Company: George Mason U
-- Engineer: Kainoa Asse
-- Create Date: 04/22/2024 10:22:24 AM
-- Revised:     02/24/2026 (Documentation and Final Verification)
-- Design Name: Sorting core on FPro System
-- Module Name: RAM - Behavioral
-- Target Devices: Basys 3 (Artix-7)
-- Tool Versions: Vivado 2023.x / Vitis 2023.x
----------------------------------------------------------------------------------

-- Dual-port RAM with synchronous read in NO_CHANGE mode
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity RAM is  
	Port(
    clk   : in  std_logic;
    wea   : in  std_logic;
    web   : in  std_logic;
    addra : in  std_logic_vector(12 downto 0);
    addrb : in  std_logic_vector(12 downto 0);
    dina   : in  std_logic_vector(15 downto 0);
    dinb   : in  std_logic_vector(15 downto 0);
    douta   : out std_logic_vector(15 downto 0);
    doutb   : out std_logic_vector(15 downto 0)
    );
end RAM;

architecture Behavioral of RAM is
    type ram_type is array (0 to 2**13-1) of std_logic_vector(15 downto 0);   
    shared variable RAM : ram_type := (others => (others => '0'));
begin
    process (clk)   
  begin    
    if rising_edge(clk) then
        if (wea = '1') then   
           RAM(to_integer(unsigned(addra))) := dina;
		else 
			douta <= RAM(to_integer(unsigned(addra)));
        end if; 			
    end if;   
  end process; 
  
  process (clk)   
  begin   
    if rising_edge(clk) then 
        if (web = '1') then   
           RAM(to_integer(unsigned(addrb))) := dinb;
		else
			doutb <= RAM(to_integer(unsigned(addrb)));
        end if; 			
    end if;   
  end process; 

end Behavioral;
